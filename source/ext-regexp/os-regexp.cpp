#include "../objectscript.h"
#include "../os-binder.h"
#include "os-regexp.h"
#include "pcre.h"

#define REGEXP_PATTERN_ORDER			1
#define REGEXP_SET_ORDER				2
#define REGEXP_OFFSET_CAPTURE			(1<<8)

#define REGEXP_GLOBAL					(1<<0)

namespace ObjectScript {

class RegExpOS: public OS
{
public:

	static void triggerError(OS * os, const OS::String& msg)
	{
		os->getGlobal(OS_TEXT("RegExpException"));
		os->pushGlobals();
		os->pushString(msg);
		os->call(1, 1);
		os->setException();
	}

	static void triggerError(OS * os, const OS_CHAR * msg)
	{
		triggerError(os, OS::String(os, msg));
	}

	struct RegExpCache
	{
		OS * os;
		pcre * re;
		pcre_extra *extra;
		int compile_options;
		int regexp_options;
		int ref_count;
#if HAVE_SETLOCALE
		need to be implemented?
		char *locale;
		unsigned const char *tables;
#endif
		RegExpCache(OS * p_os)
		{
			os = p_os;
			re = NULL;
			extra = NULL;
			regexp_options = 0;
			compile_options = 0;
			ref_count = 1;
		}

		~RegExpCache()
		{
			pcre_free(re);
			if (extra) pcre_free(extra);
#if HAVE_SETLOCALE
			if ((void*)pce->tables) pefree((void*)pce->tables, 1);
			pefree(pce->locale, 1);
#endif
		}

		RegExpCache * retain()
		{
			ref_count++;
			return this;
		}

		void release()
		{
			if(--ref_count <= 0){
				OS_ASSERT(ref_count == 0);
				OS * os = this->os;
				this->~RegExpCache();
				os->free(this);
			}		
		}

		static void initLibrary(OS * os);
	};

	static RegExpCache * toRegExpCache(OS*);

	struct RegExp
	{
		RegExpCache * cache;
		
		RegExp(RegExpCache * p_cache)
		{
			OS_ASSERT(p_cache);
			cache = p_cache->retain();
		}

		~RegExp()
		{
			OS_ASSERT(cache);
			cache->release();
		}

		static bool isNumericString(const OS_CHAR * str)
		{
			for(; *str; str++){
				if(!OS_IS_ALNUM(*str)){
					return false;
				}
			}
			return true;
		}

		char **makeSubpatsTable(int num_subpats)
		{
			OS_ASSERT(cache);
			OS * os = cache->os;
			pcre_extra *extra = cache->extra;
			int name_cnt = 0, name_size, ni = 0;
			char *name_table;
			unsigned short name_idx;
			char **subpat_names = (char **)os->malloc(num_subpats * sizeof(char *) OS_DBG_FILEPOS);

			int rc = pcre_fullinfo(cache->re, extra, PCRE_INFO_NAMECOUNT, &name_cnt);
			if (rc < 0) {
				triggerError(os, OS::String::format(os, "PCRE fullinfo (makeSubpatsTable) error code %d", rc));
				os->free(subpat_names);
				return NULL;
			}
			if (name_cnt > 0) {
				int rc1 = pcre_fullinfo(cache->re, extra, PCRE_INFO_NAMETABLE, &name_table);
				int rc2 = pcre_fullinfo(cache->re, extra, PCRE_INFO_NAMEENTRYSIZE, &name_size);
				rc = rc2 ? rc2 : rc1;
				if (rc < 0) {
					triggerError(os, OS::String::format(os, "PCRE fullinfo (makeSubpatsTable #2) error code %d", rc));
					os->free(subpat_names);
					return NULL;
				}

				while (ni++ < name_cnt) {
					name_idx = 0xff * (unsigned char)name_table[0] + (unsigned char)name_table[1];
					subpat_names[name_idx] = name_table + 2;
					if (isNumericString(subpat_names[name_idx])) {
						triggerError(os, "PCRE numeric named subpatterns are not allowed");
						os->free(subpat_names);
						return NULL;
					}
					name_table += name_size;
				}
			}
			return subpat_names;
		}

		void addNextString(const char * str, int len)
		{
			OS_ASSERT(cache);
			OS * os = cache->os;
			os->pushStackValue();
			os->pushString(str, len);
			os->addProperty(false);
		}

		void addNextNumber(int value)
		{
			OS_ASSERT(cache);
			OS * os = cache->os;
			os->pushStackValue();
			os->pushNumber(value);
			os->addProperty(false);
		}

		void addOffsetPair(const char *str, int len, int offset, const char *name)
		{
			OS_ASSERT(cache);
			OS * os = cache->os;
			os->newArray();

			/* Add (match, offset) to the return value */
			addNextString(str, len);
			addNextNumber(offset);

			if (name) {
				os->pushStackValue(-2);
				os->pushStackValue(-2);
				os->setProperty(name, false);
			}
			os->pushStackValue(-2);
			os->pushStackValue(-2);
			os->addProperty(false);

			os->pop();
		}

		void addAssocString(const char * name, const char *str, int len)
		{
			OS_ASSERT(cache);
			OS * os = cache->os;
			os->pushStackValue();
			os->pushString(str, len);
			os->setProperty(name, false);
		}

		bool checkExecError(int pcre_code)
		{
			OS_ASSERT(cache);
			OS * os = cache->os;
			switch (pcre_code) {
				case PCRE_ERROR_MATCHLIMIT:
					triggerError(os, "PCRE backtrack limit error");
					return true;

				case PCRE_ERROR_RECURSIONLIMIT:
					triggerError(os, "PCRE recursion limit error");
					return true;

				case PCRE_ERROR_BADUTF8:
					triggerError(os, "PCRE bad utf8 error");
					return true;

				case PCRE_ERROR_BADUTF8_OFFSET:
					triggerError(os, "PCRE bad utf8 offset error");
					return true;

				default:
					triggerError(os, OS::String::format(os, "PCRE internal error code %d", pcre_code));
					return true;
			}
			return false;
		}

		static RegExpCache * getRegExpCache(OS * os, const Core::String& regex)
		{
			pcre				*re = NULL;
			pcre_extra			*extra;
			int					 compile_options = 0;
			int					 soptions = 0;
			const char			*error;
			int					 erroffset;
			char				 delimiter;
			char				 start_delimiter;
			char				 end_delimiter;
			const char			*p, *pp;
			// const char			*pattern;
			int					 do_study = 0;
			int					 regexp_options = 0;
			int					 count = 0;
			unsigned const char *tables = NULL;
#if HAVE_SETLOCALE
			char				*locale = setlocale(LC_CTYPE, NULL);
#endif

#if 1
			/* Try to lookup the cached regex entry, and if successful, just pass
			back the compiled pattern, otherwise go on and compile it. */
			os->getGlobal("RegExp");
			os->getProperty("cache", false);
			if(os->isNull()){
				os->newObject();
				os->getGlobal("RegExp");
				os->pushStackValue(-2);
				os->setProperty("cache", false);
			}
			int cache_id = os->getValueId();
			os->getProperty(regex, false);
			RegExpCache * cache = toRegExpCache(os);
			if(cache){
				/*
				* We use a quick pcre_fullinfo() check to see whether cache is corrupted, and if it
				* is, we flush it and compile the pattern from scratch.
				*/
				if (pcre_fullinfo(cache->re, NULL, PCRE_INFO_CAPTURECOUNT, &count) == PCRE_ERROR_BADMAGIC) {
					os->pushValueById(cache_id);
					os->deleteProperty(regex, false);
					os->pop();
				} else {
#if HAVE_SETLOCALE
					if (!strcmp(cache->locale, locale)) {
#endif
					// touch item
					os->pushValueById(cache_id);
					os->deleteProperty(regex, false);
					
					os->pushValueById(cache_id);
					os->pushStackValue(-2); // pushCtypeValue(os, cache);
					os->setProperty(regex, false);

					os->pop();
					return cache;
#if HAVE_SETLOCALE
					}
#endif
				}
			}
			os->pop();
#endif

			p = regex;

			/* Parse through the leading whitespace, and display a warning if we
			get to the end without encountering a delimiter. */
			while (OS_IS_SPACE(*p)) p++;
			if (*p == 0) {
				triggerError(os, "PCRE empty regular expression");
				return NULL;
			}

			/* Get the delimiter and display a warning if it is alphanumeric
			or a backslash. */
			delimiter = *p++;
			if (OS_IS_ALNUM(delimiter) || delimiter == '\\') {
				triggerError(os, "PCRE delimiter must not be alphanumeric or backslash");
				return NULL;
			}

			start_delimiter = delimiter;
			if ((pp = OS_STRCHR("([{< )]}> )]}>", delimiter)))
				delimiter = pp[5];
			end_delimiter = delimiter;

			if (start_delimiter == end_delimiter) {
				/* We need to iterate through the pattern, searching for the ending delimiter,
				but skipping the backslashed delimiters.  If the ending delimiter is not
				found, display a warning. */
				pp = p;
				while (*pp != 0) {
					if (*pp == '\\' && pp[1] != 0) pp++;
					else if (*pp == delimiter)
						break;
					pp++;
				}
				if (*pp == 0) {
					triggerError(os, OS::String::format(os, "PCRE no ending delimiter '%c' found", delimiter));
					return NULL;
				}
			} else {
				/* We iterate through the pattern, searching for the matching ending
				* delimiter. For each matching starting delimiter, we increment nesting
				* level, and decrement it for each matching ending delimiter. If we
				* reach the end of the pattern without matching, display a warning.
				*/
				int brackets = 1; 	/* brackets nesting level */
				pp = p;
				while (*pp != 0) {
					if (*pp == '\\' && pp[1] != 0) pp++;
					else if (*pp == end_delimiter && --brackets <= 0)
						break;
					else if (*pp == start_delimiter)
						brackets++;
					pp++;
				}
				if (*pp == 0) {
					triggerError(os, OS::String::format(os, "PCRE no ending matching delimiter '%c' found", end_delimiter));
					return NULL;
				}
			}

			/* Make a copy of the actual pattern. */
			Core::String pattern(os, p, pp-p);

			/* Move on to the options */
			pp++;

			/* Parse through the options, setting appropriate flags.  Display
			a warning if we encounter an unknown modifier. */	
			while (*pp != 0) {
				switch (*pp++) {
					/* Perl compatible options */
				case 'i':	compile_options |= PCRE_CASELESS;		break;
				case 'm':	compile_options |= PCRE_MULTILINE;		break;
				case 's':	compile_options |= PCRE_DOTALL;		break;
				case 'x':	compile_options |= PCRE_EXTENDED;		break;

					/* PCRE specific options */
				case 'A':	compile_options |= PCRE_ANCHORED;		break;
				case 'D':	compile_options |= PCRE_DOLLAR_ENDONLY;break;
				case 'S':	do_study  = 1;					break;
				case 'U':	compile_options |= PCRE_UNGREEDY;		break;
				case 'X':	compile_options |= PCRE_EXTRA;			break;
				case 'u':	compile_options |= PCRE_UTF8;
					/* In  PCRE,  by  default, \d, \D, \s, \S, \w, and \W recognize only ASCII
					characters, even in UTF-8 mode. However, this can be changed by setting
					the PCRE_UCP option. */
#ifdef PCRE_UCP
					compile_options |= PCRE_UCP;
#endif			
					break;

					/* Custom preg options */
				// case 'e':	regexp_options |= REGEXP_REPLACE_EVAL;	break;
				case 'g': regexp_options |= REGEXP_GLOBAL; break;

				/*
				case ' ':
				case '\n':
					break;
				*/

				default:
					triggerError(os, OS::String::format(os, "PCRE unknown modifier '%c'", pp[-1]));
					return NULL;
				}
			}

#if HAVE_SETLOCALE
			if (strcmp(locale, "C"))
				tables = pcre_maketables();
#endif

			/* Compile pattern and display a warning if compilation failed. */
			re = pcre_compile(pattern,
				compile_options,
				&error,
				&erroffset,
				tables);

			if (re == NULL) {
				triggerError(os, OS::String::format(os, "PCRE compilation failed: %s at offset %d", error, erroffset));
				if (tables) {
					os->free((void*)tables);
				}
				return NULL;
			}

			/* If study option was specified, study the pattern and
			store the result in extra for passing to pcre_exec. */
			if (do_study) {
				extra = pcre_study(re, soptions, &error);
				if (extra) {
					extra->flags |= PCRE_EXTRA_MATCH_LIMIT | PCRE_EXTRA_MATCH_LIMIT_RECURSION;
				}
				if (error != NULL) {
					triggerError(os, "PCRE error while studying pattern");
					return NULL;
				}
			} else {
				extra = NULL;
			}

			/*
			* If we reached cache limit, clean out the items
			*/
			os->pushValueById(cache_id);
			count = os->getLen();
			for(; count > 512 && os->nextIteratorStep(); count--){
				os->pushValueById(cache_id);
				os->pushStackValue(-3);
				os->deleteProperty(false);
				os->pop(2);
			}

			/* Store the compiled pattern and extra info in the cache. */
			cache = new (os->malloc(sizeof(*cache) OS_DBG_FILEPOS)) RegExpCache(os); 
			cache->re = re;
			cache->extra = extra;
			cache->regexp_options = regexp_options;
			cache->compile_options = compile_options;
#if HAVE_SETLOCALE
			new_entry.locale = pestrdup(locale, 1);
			new_entry.tables = tables;
#endif

#if 1
			os->pushValueById(cache_id);
			pushCtypeValue(os, cache);
			os->setProperty(regex, false);
#endif
			return cache;
		}

		bool match(const OS::String& subject, int subpats_id, int use_flags, int flags, int start_offset)
		{
			OS_ASSERT(cache);
			OS * os = cache->os;
			pcre_extra * extra = cache->extra;	/* Holds results of studying */
			pcre_extra		 extra_data;		/* Used locally for exec options */
			int				 exoptions = 0;		/* Execution options */
			int				 count = 0;			/* Count of matched subpatterns */
			int				*offsets;			/* Array of subpattern offsets */
			int				 num_subpats;		/* Number of captured subpatterns */
			int				 size_offsets;		/* Size of the offsets array */
			int				 matched;			/* Has anything matched */
			int				 g_notempty = 0;	/* If the match should not be empty */
			const char	   **stringlist;		/* Holds list of subpatterns */
			char 		   **subpat_names;		/* Array for named subpatterns */
			int				 i, rc;
			int				 subpats_order;		/* Order of subpattern matches */
			int				 offset_capture;    /* Capture match offsets: yes/no */

			/* RegExpCache * pce = getRegExpCache(subject);
			if(!pce){
				return false;
			} */

			int global = cache->regexp_options & REGEXP_GLOBAL;
			subpats_order = global ? REGEXP_PATTERN_ORDER : 0;

			if (use_flags) {
				offset_capture = flags & REGEXP_OFFSET_CAPTURE;

				/*
				* subpats_order is pre-set to pattern mode so we change it only if
				* necessary.
				*/
				if (flags & 0xff) {
					subpats_order = flags & 0xff;
				}
				if ((global && (subpats_order < REGEXP_PATTERN_ORDER || subpats_order > REGEXP_SET_ORDER)) ||
					(!global && subpats_order != 0)) {
						triggerError(os, "PCRE invalid flag specified");
						return false;
				}
			} else {
				offset_capture = 0;
			}

			/* Negative offset counts from the end of the string. */
			int subject_len = subject.getLen();
			if (start_offset < 0) {
				start_offset = subject_len + start_offset;
				if (start_offset < 0) {
					start_offset = 0;
				}
			}

			if (extra == NULL) {
				extra_data.flags = PCRE_EXTRA_MATCH_LIMIT | PCRE_EXTRA_MATCH_LIMIT_RECURSION;
				extra = &extra_data;
			}

			os->getGlobal("RegExp");
			extra->match_limit			 = (os->getProperty(-1, "backtrackLimit"), os->popInt());
			extra->match_limit_recursion = (os->getProperty(-1, "recursionLimit"), os->popInt());
			os->pop(); // RegExp

			/* Calculate the size of the offsets array, and allocate memory for it. */
			rc = pcre_fullinfo(cache->re, extra, PCRE_INFO_CAPTURECOUNT, &num_subpats);
			if (rc < 0) {
				triggerError(os, OS::String::format(os, "PCRE fullinfo error code %d", rc));
				return false;
			}
			num_subpats++;
			size_offsets = num_subpats * 3;

			/*
			* Build a mapping from subpattern numbers to their names. We will always
			* allocate the table, even though there may be no named subpatterns. This
			* avoids somewhat more complicated logic in the inner loops.
			*/
			subpat_names = makeSubpatsTable(num_subpats);
			if (!subpat_names) {
				return false;
			}

			offsets = (int *)os->malloc(size_offsets * sizeof(int) OS_DBG_FILEPOS);

			/* Allocate match sets array and initialize the values. */
			struct MatchSets {
				OS * os;
				int * sets;
				int num_subpats;

				MatchSets(OS * p_os)
				{
					os = p_os;
					sets = NULL;
					num_subpats = 0;
				}
				~MatchSets()
				{
					for(int i = 0; i < num_subpats; i++){
						os->releaseValueById(sets[i]);
					}
					os->free(sets);
				}

				void alloc(int p_num_subpats)
				{
					num_subpats = p_num_subpats;
					sets = (int*)os->malloc(num_subpats * sizeof(int) OS_DBG_FILEPOS);
					for (int i = 0; i < num_subpats; i++) {
						os->newObject();
						sets[i] = os->getValueId();
						os->retainValueById(sets[i]);
						os->pop();
					}
				}

			} match_sets(os);
			
			if (global && subpats_id && subpats_order == REGEXP_PATTERN_ORDER) {
				match_sets.alloc(num_subpats);
			}

			matched = 0;
			// PCRE_G(error_code) = PHP_PCRE_NO_ERROR;

			do {
				/* Execute the regular expression. */
				count = pcre_exec(cache->re, extra, subject, subject_len, start_offset,
					exoptions|g_notempty, offsets, size_offsets);

				/* the string was already proved to be valid UTF-8 */
				exoptions |= PCRE_NO_UTF8_CHECK;

				/* Check for too many substrings condition. */
				if (count == 0) {
					triggerError(os, "PCRE matched, but too many substrings");
					return false;
					// count = size_offsets/3;
				}

				/* If something has matched */
				if (count > 0) {
					matched++;

					/* If subpatterns array has been passed, fill it in with values. */
					if (subpats_id) {
						/* Try to get the list of substrings and display a warning if failed. */
						if (pcre_get_substring_list(subject, offsets, count, &stringlist) < 0) {
							os->free(subpat_names);
							os->free(offsets);
							// if (match_sets) efree(match_sets);
							triggerError(os, "PCRE get subpatterns list failed");
							return false;
						}

						if (global) {	/* global pattern matching */
							if (subpats_id && subpats_order == REGEXP_PATTERN_ORDER) {
								/* For each subpattern, insert it into the appropriate array. */
								for (i = 0; i < count; i++) {
									os->pushValueById(match_sets.sets[i]);
									if (offset_capture) {
										addOffsetPair((char *)stringlist[i],
											offsets[(i<<1)+1] - offsets[i<<1], offsets[i<<1], NULL);
									} else {
										addNextString((char *)stringlist[i],
											offsets[(i<<1)+1] - offsets[i<<1]);
									}
									os->pop();
								}
								/*
								* If the number of captured subpatterns on this run is
								* less than the total possible number, pad the result
								* arrays with empty strings.
								*/
								if (count < num_subpats) {
									for (; i < num_subpats; i++) {
										os->pushValueById(match_sets.sets[i]);
										os->pushString("");
										os->addProperty(false);
										// addNextString("", 0);
										// os->pop();
									}
								}
							} else {
								/* Allocate the result set array */
								os->newObject();
								
								/* Add all the subpatterns to it */
								for (i = 0; i < count; i++) {
									if (offset_capture) {
										addOffsetPair((char *)stringlist[i],
											offsets[(i<<1)+1] - offsets[i<<1], offsets[i<<1], subpat_names[i]);
									} else {
										if (subpat_names[i]) {
											addAssocString(subpat_names[i], (char *)stringlist[i],
												offsets[(i<<1)+1] - offsets[i<<1]);
										}
										addNextString((char *)stringlist[i],
											offsets[(i<<1)+1] - offsets[i<<1]);
									}
								}
								/* And add it to the output array */
								os->pushValueById(subpats_id);
								os->pushStackValue(-2);
								os->addProperty(false);

								os->pop();
							}
						} else {			/* single pattern matching */
							/* For each subpattern, insert it into the subpatterns array. */
							os->pushValueById(subpats_id);
							for (i = 0; i < count; i++) {
								if (offset_capture) {
									addOffsetPair((char *)stringlist[i],
										offsets[(i<<1)+1] - offsets[i<<1],
										offsets[i<<1], subpat_names[i]);
								} else {
									if (subpat_names[i]) {
										addAssocString(subpat_names[i], (char *)stringlist[i],
											offsets[(i<<1)+1] - offsets[i<<1]);
									}
									addNextString((char *)stringlist[i],
										offsets[(i<<1)+1] - offsets[i<<1]);
								}
							}
							os->pop();
						}

						pcre_free((void *) stringlist);
					}
				} else if (count == PCRE_ERROR_NOMATCH) {
					/* If we previously set PCRE_NOTEMPTY after a null match,
					this is not necessarily the end. We need to advance
					the start offset, and continue. Fudge the offset values
					to achieve this, unless we're already at the end of the string. */
					if (g_notempty != 0 && start_offset < subject_len) {
						offsets[0] = start_offset;
						offsets[1] = start_offset + 1;
					} else
						break;
				} else {
					if(checkExecError(count)){
						return false;
					}
					OS_ASSERT(false);
					break;
				}

				/* If we have matched an empty string, mimic what Perl's /g options does.
				This turns out to be rather cunning. First we set PCRE_NOTEMPTY and try
				the match again at the same point. If this fails (picked up above) we
				advance to the next character. */
				g_notempty = (offsets[1] == offsets[0])? PCRE_NOTEMPTY | PCRE_ANCHORED : 0;

				/* Advance to the position right after the last full match */
				start_offset = offsets[1];
			} while (global);

			/* Add the match sets to the output array and clean up */
			if (global && subpats_id && subpats_order == REGEXP_PATTERN_ORDER) {
				os->pushValueById(subpats_id);
				for (i = 0; i < num_subpats; i++) {
					if (subpat_names[i]) {
						os->pushStackValue();
						os->pushValueById(match_sets.sets[i]);
						os->setProperty(subpat_names[i], false);
					}
					os->pushStackValue();
					os->pushValueById(match_sets.sets[i]);
					os->addProperty(false);
				}
				os->pop();
				// efree(match_sets);
			}

			os->free(offsets);
			os->free(subpat_names);

			return matched > 0;
		}

		static bool getBackRef(const char **str, int *backref)
		{
			const char *walk = *str;
			OS_ASSERT(*walk == '$');
			if (walk[1] == 0)
				return false;

			walk++;
			if(OS_IS_ALNUM(*walk)){
				*backref = *walk - '0';
				walk++;
			} else
				return false;

			if(*walk && OS_IS_ALNUM(*walk)){
				*backref = *backref * 10 + *walk - '0';
				walk++;
			}

			*str = walk;
			return true;	
		}

		OS::String callbackReplace(int function_id, const char *subject, int *offsets, char **subpat_names, int count, const OS::String& subject_str)
		{
			OS_ASSERT(cache);
			OS * os = cache->os;

			os->pushValueById(function_id);
			OS_ASSERT(os->isFunction());
			os->pushNull(); // this for function

			os->newObject();
			for (int i = 0; i < count; i++) {
				if (subpat_names[i]) {
					addAssocString(subpat_names[i], &subject[offsets[i<<1]] , offsets[(i<<1)+1] - offsets[i<<1]);
				}
				addNextString(&subject[offsets[i<<1]], offsets[(i<<1)+1] - offsets[i<<1]);
			}
			os->pushString(subject_str);
			os->call(2, 1);
			return os->popString();
		}

		bool replace(const OS::String& subject_str, int replace_id, int limit, int& replace_count)
		{
			OS_ASSERT(cache);
			OS * os = cache->os;
			pcre_extra		*extra = cache->extra;/* Holds results of studying */
			pcre_extra		 extra_data;		/* Used locally for exec options */
			int				 exoptions = 0;		/* Execution options */
			int				 count = 0;			/* Count of matched subpatterns */
			int				*offsets;			/* Array of subpattern offsets */
			char 			**subpat_names;		/* Array for named subpatterns */
			int				 num_subpats;		/* Number of captured subpatterns */
			int				 size_offsets;		/* Size of the offsets array */
			int				 backref;			/* Backreference number */
			int				 start_offset;		/* Where the new search starts */
			int				 g_notempty=0;		/* If the match should not be empty */
			int				 replace_len=0;		/* Length of replacement string */
			const char * replace = NULL;		/* Replacement string */
			const char * replace_end = NULL;	/* End of replacement string */
			int				 rc;

			if (extra == NULL) {
				extra_data.flags = PCRE_EXTRA_MATCH_LIMIT | PCRE_EXTRA_MATCH_LIMIT_RECURSION;
				extra = &extra_data;
			}

			os->getGlobal("RegExp");
			extra->match_limit			 = (os->getProperty(-1, "backtrackLimit"), os->popInt());
			extra->match_limit_recursion = (os->getProperty(-1, "recursionLimit"), os->popInt());
			os->pop(); // RegExp

			os->pushValueById(replace_id);
			bool is_callable_replace = os->isFunction();
			OS::String replace_str = is_callable_replace ? OS::String(os) : os->toString();
			if(!is_callable_replace){
				replace = replace_str.toChar();
				replace_len = replace_str.getLen();
				replace_end = replace + replace_len;
			}
			os->pop(); // replace_id

			/* Calculate the size of the offsets array, and allocate memory for it. */
			rc = pcre_fullinfo(cache->re, extra, PCRE_INFO_CAPTURECOUNT, &num_subpats);
			if (rc < 0) {
				triggerError(os, OS::String::format(os, "PCRE internal pcre_fullinfo() error %d", rc));
				return false;
			}
			num_subpats++;
			size_offsets = num_subpats * 3;

			/*
			* Build a mapping from subpattern numbers to their names. We will always
			* allocate the table, even though there may be no named subpatterns. This
			* avoids somewhat more complicated logic in the inner loops.
			*/
			subpat_names = makeSubpatsTable(num_subpats);
			if (!subpat_names) {
				return false;
			}

			offsets = (int *)os->malloc(size_offsets * sizeof(int) OS_DBG_FILEPOS);

			Core::Buffer result(os);

			const char * subject = subject_str;
			int subject_len = subject_str.getLen();
			result.reserveCapacity((2 * subject_len + 1)*sizeof(char));

			/* Initialize */
			replace_count = 0;
			start_offset = 0;
			
			while (1) {
				/* Execute the regular expression. */
				count = pcre_exec(cache->re, extra, subject, subject_len, start_offset,
					exoptions|g_notempty, offsets, size_offsets);

				/* the string was already proved to be valid UTF-8 */
				exoptions |= PCRE_NO_UTF8_CHECK;

				/* Check for too many substrings condition. */
				if (count == 0) {
					triggerError(os, "PCRE matched, but too many substrings");
					return false;
					// count = size_offsets/3;
				}

				const char * piece = subject + start_offset;

				if (count > 0 && (limit == -1 || limit > 0)) {
					++replace_count;
					
					/* Set the match location in subject */
					const char * match = subject + offsets[0];

					/* copy the part of the string before the match */
					result.append(piece, match-piece);

					/* If evaluating or using custom function, copy result to the buffer
					* and clean up. */
					if (is_callable_replace) {
						OS::String callback_result = callbackReplace(replace_id, subject, offsets, subpat_names, count, subject_str);
						result.append(callback_result);
					} else { /* do regular backreference copying */
						const char * walk = replace;
						while (walk < replace_end) {
							if ('$' == *walk) {
								// $&	Inserts the matched substring (not implemented yet)
								// $`	Inserts the portion of the string that precedes the matched substring (not implemented yet)
								// $'	Inserts the portion of the string that follows the matched substring (not implemented yet)
								// $$	Inserts a "$"
								if(walk[1] == '$'){
									result.append(walk, 1);
									walk += 2;
									continue;
								}else if(getBackRef(&walk, &backref)){
									if (backref < count){
										int match_len = offsets[(backref<<1)+1] - offsets[backref<<1];
										result.append(subject + offsets[backref<<1], match_len);
									}
									continue;
								}
							}
							result.append(walk++, 1);
						}
					}

					if (limit != -1)
						limit--;

				} else if (count == PCRE_ERROR_NOMATCH || limit == 0) {
					/* If we previously set PCRE_NOTEMPTY after a null match,
					this is not necessarily the end. We need to advance
					the start offset, and continue. Fudge the offset values
					to achieve this, unless we're already at the end of the string. */
					if (g_notempty != 0 && start_offset < subject_len) {
						offsets[0] = start_offset;
						offsets[1] = start_offset + 1;
						result.append(piece, 1);
					} else {
						/* stick that last bit of string on our output */
						result.append(piece, subject_len - start_offset);
						break;
					}
				} else {
					if(checkExecError(count)){
						return false;
					}
					OS_ASSERT(false);
					break;
				}

				/* If we have matched an empty string, mimic what Perl's /g options does.
				This turns out to be rather cunning. First we set PCRE_NOTEMPTY and try
				the match again at the same point. If this fails (picked up above) we
				advance to the next character. */
				g_notempty = (offsets[1] == offsets[0])? PCRE_NOTEMPTY | PCRE_ANCHORED : 0;

				/* Advance to the next piece. */
				start_offset = offsets[1];
			}

			os->free(offsets);
			os->free(subpat_names);

			os->pushString(result);
			return true;
		}

		static void initLibrary(OS * os);
	};

	static void initLibrary(OS * os)
	{
#define OS_AUTO_TEXT(exp) OS_TEXT(#exp)
		os->eval(OS_AUTO_TEXT(
			RegExpException = extends Exception {
			}
		));
	}
};

template <> struct CtypeName<RegExpOS::RegExp>{ static const OS_CHAR * getName(){ return OS_TEXT("RegExp"); } };
template <> struct CtypeValue<RegExpOS::RegExp*>: public CtypeUserClass<RegExpOS::RegExp*>{};
template <> struct UserDataDestructor<RegExpOS::RegExp>
{
	static void dtor(ObjectScript::OS * os, void * data, void * user_param)
	{
		OS_ASSERT(data && dynamic_cast<RegExpOS::RegExp*>((RegExpOS::RegExp*)data));
		RegExpOS::RegExp * obj = (RegExpOS::RegExp*)data;
		obj->~RegExp();
		os->free(obj);
	}
};

template <> struct CtypeName<RegExpOS::RegExpCache>{ static const OS_CHAR * getName(){ return OS_TEXT("RegExpCache"); } };
template <> struct CtypeValue<RegExpOS::RegExpCache*>: public CtypeUserClass<RegExpOS::RegExpCache*>{};
template <> struct UserDataDestructor<RegExpOS::RegExpCache>
{
	static void dtor(ObjectScript::OS * os, void * data, void * user_param)
	{
		OS_ASSERT(data && dynamic_cast<RegExpOS::RegExpCache*>((RegExpOS::RegExpCache*)data));
		RegExpOS::RegExpCache * buf = (RegExpOS::RegExpCache*)data;
		buf->release();
		// buf->~RegExpCache();
		// os->free(buf);
	}
};

RegExpOS::RegExpCache * RegExpOS::toRegExpCache(OS * os)
{
	return CtypeValue<RegExpOS::RegExpCache*>::getArg(os, -1);
}			

void RegExpOS::RegExpCache::initLibrary(OS * os)
{
	struct Lib
	{
		static void construct(OS * os)
		{
			triggerError(os, OS_TEXT("you should not create new instance of RegExpCache"));
		}
	};

	OS::FuncDef funcs[] = {
		def(OS_TEXT("__construct"), Lib::construct),
		{}
	};

	registerUserClass<RegExpCache>(os, funcs);
}

void RegExpOS::RegExp::initLibrary(OS * os)
{
	struct Lib
	{
		static RegExp * construct(OS * os, const OS::String& pattern)
		{
			RegExpCache * cache = getRegExpCache(os, pattern);
			if(!cache){
				return NULL;
			}
			return new (os->malloc(sizeof(RegExp) OS_DBG_FILEPOS)) RegExp(cache);
		}

		static int exec(OS * os, int params, int, int, void * user_param)
		{
			OS_GET_SELF(RegExp*);

			if(params < 1){
				triggerError(os, "argument required");
				return 0;
			}
			OS::String subject = os->toString(-params+0);

			bool use_flags = params > 1;
			int flags = REGEXP_PATTERN_ORDER;
			int offset = 0;
			if(use_flags){
				flags = os->toInt(-params+1);
				if(params > 2){
					offset = os->toInt(-params+2);
				}
			}

			// os->newArray();
			os->newObject();
			return self->match(subject, os->getValueId(), use_flags, flags, offset) ? 1 : 0;
		}

		static int test(OS * os, int params, int, int, void * user_param)
		{
			OS_GET_SELF(RegExp*);

			if(params < 1){
				triggerError(os, "argument required");
				return 0;
			}
			OS::String subject = os->toString(-params+0);
			os->pushBool(self->match(subject, 0, false, 0, 0));
			return 1;
		}

		static int replace(OS * os, int params, int, int, void * user_param)
		{
			OS_GET_SELF(RegExp*);

			if(params < 2){
				triggerError(os, "two arguments required");
				return 0;
			}
			OS::String subject = os->toString(-params+0);
			int replace_id = os->getValueId(-params+1);
			int limit = params >= 3 ? os->toInt(-params+2, -1) : -1;
			int replace_count = 0;
			return self->replace(subject, replace_id, limit, replace_count) ? 1 : 0;
		}
	};

	OS::FuncDef funcs[] = {
		def(OS_TEXT("__construct"), Lib::construct),
		{OS_TEXT("exec"), Lib::exec},
		{OS_TEXT("test"), Lib::test},
		{OS_TEXT("replace"), Lib::replace},
		{}
	};

	OS::NumberDef numbers[] = {
		{OS_TEXT("PATTERN_ORDER"), REGEXP_PATTERN_ORDER},
		{OS_TEXT("SET_ORDER"), REGEXP_SET_ORDER},
		{OS_TEXT("OFFSET_CAPTURE"), REGEXP_OFFSET_CAPTURE},
		{OS_TEXT("backtrackLimit"), 1000000},
		{OS_TEXT("recursionLimit"), 100000},
		{}
	};

	registerUserClass<RegExp>(os, funcs, numbers);
}

void initRegExpLibrary(OS* os)
{
	RegExpOS::RegExp::initLibrary(os);
	RegExpOS::RegExpCache::initLibrary(os);
	RegExpOS::initLibrary(os);
}

} // namespace ObjectScript
