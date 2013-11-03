require "../std"

function __get(name){
	print "global property \"${name}\" is not declared"
	print "back trace"
	printBackTrace(1)
	print ""
}

function printBackTrace(skipNumFuncs){
	for(var i, t in debugBackTrace(skipNumFuncs + 1)){ // skip printBackTrace
		printf("#${i} ${t.file}%s: %s, args: ${t.arguments}\n",
			t.line > 0 ? "(${t.line},${t.pos})" : "",
			t.object && t.object !== _G ? "<${typeOf(t.object)}#${t.object.__id}>.${t.__name}" : t.__name)
	}
}

var SER_COUNT = 240;

function r()
{
  var r = 0;
  for (var i = 0; i < SER_COUNT; ++i)
    if (math.random(2) == 0)
      r++;
  return r;
}

function display(){
  glClear(GL_COLOR_BUFFER_BIT);
  var q = [];
  for (var i = 0; i <= SER_COUNT; ++i)
    q[i] = 0;
  for (var i = 0; i < 10000; ++i){
    var j = r();
	q[j] = q[j]+1;
  }
  var max = 0;
  for (var i = 0; i <= SER_COUNT; ++i)
    if (max < q[i])
      max = q[i];
	  
  glBegin(GL_LINE_STRIP);
  glColor3f(0.0, 0.0, 0.0);
  for (var i = 0; i <= SER_COUNT; ++i)
    glVertex2f(i, SER_COUNT * q[i] / max);
  glEnd();
  
  print q;

  glutSwapBuffers();
}

KEY_ESCAPE = 27

function keyboard(key, mousePositionX, mousePositionY){ 
	if(key == KEY_ESCAPE){        
		process.exit(0);
	}
}

// initialize and run program
glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);  // Display Mode
glutInitWindowSize(240, 240);					// set window size
glutCreateWindow("OpenGL/GLUT Example 06");								// create Window

glClearColor(1.0, 1.0, 1.0, 1.0);

glMatrixMode(GL_PROJECTION);
glLoadIdentity();
glOrtho(0.0, 240.0, 0.0, 240.0, -1.0, 1.0);
glutDisplayFunc(display);
glutKeyboardFunc(keyboard);
glutMainLoop();
	