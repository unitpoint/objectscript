// see http://www.php.net/manual/ru/function.curl-version.php

var version = Curl.VERSION

bitfields = {
    ipv6          = Curl.FEATURE_IPV6,
    kerberos64    = Curl.FEATURE_KERBEROS4,
    ssl           = Curl.FEATURE_SSL,
    libz          = Curl.FEATURE_LIBZ,
    ntlm          = Curl.FEATURE_NTLM,
    gssnegotiate  = Curl.FEATURE_GSSNEGOTIATE,
    debug         = Curl.FEATURE_DEBUG,
    asynchdns     = Curl.FEATURE_ASYNCHDNS,
    spnego        = Curl.FEATURE_SPNEGO,
    largefile     = Curl.FEATURE_LARGEFILE,
    idn           = Curl.FEATURE_IDN,
    sspi          = Curl.FEATURE_SSPI,
    conv          = Curl.FEATURE_CONV,
    curldebug     = Curl.FEATURE_CURLDEBUG,
    tlsauth_srp   = Curl.FEATURE_TLSAUTH_SRP,
    ntlm_wb       = Curl.FEATURE_NTLM_WB,
}

for(var feature, bit in bitfields) {
    if (version["features"] & bit != 0)
        printf ("%12s: yes\n", feature)
    else
        printf ("%12s: no\n", feature)
}

print version
print bitfields
