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

win = {};
win.width = 640;
win.height = 480;
win.title = "OpenGL/GLUT Example 01";
win.field_of_view_angle = 45;
win.z_near = 1.0f;
win.z_far = 500.0f;

print win

function display(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		     // Clear Screen and Depth Buffer
	glLoadIdentity();
	glTranslatef(0.0f,0.0f,-3.0f);			
 
	/*
	 * Triangle code starts here
	 * 3 verteces, 3 colors.
	 */
	// print "GL_TRIANGLES"
	glBegin(GL_TRIANGLES);					
		glColor3f(0.0f,0.0f,1.0f);			
		glVertex3f(0.0f, 1.0f, 0.0f);		
		glColor3f(0.0f,1.0f,0.0f);			
		glVertex3f(-1.0f,-1.0f, 0.0f);		
		glColor3f(1.0f,0.0f,0.0f);			
		glVertex3f(1.0f,-1.0f, 0.0f);		
	glEnd();				
 
	glutSwapBuffers();	
}

function initialize(){
	glMatrixMode(GL_PROJECTION);												// select projection matrix
	glViewport(0, 0, win.width, win.height);									// set the viewport
	glMatrixMode(GL_PROJECTION);												// set matrix mode
	glLoadIdentity();															// reset projection matrix
	gluPerspective(win.field_of_view_angle, win.width / win.height, win.z_near, win.z_far);		// set up a perspective projection matrix
	glMatrixMode(GL_MODELVIEW);													// specify which matrix is the current matrix
	glShadeModel(GL_SMOOTH);
	glClearDepth(1.0f);														// specify the clear value for the depth buffer
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);						// specify implementation-specific hints
	glClearColor(0.0, 0.0, 0.0, 1.0);											// specify clear values for the color buffers								
}

KEY_ESCAPE = 27

function keyboard(key, mousePositionX, mousePositionY){ 
	if(key == KEY_ESCAPE){        
		process.exit(0);
	}
}

// initialize and run program
glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );  // Display Mode
glutInitWindowSize(win.width, win.height);					// set window size
glutCreateWindow(win.title);								// create Window
glutDisplayFunc(display);									// register Display Function
glutIdleFunc(display);									// register Idle Function
glutKeyboardFunc(keyboard);								// register Keyboard Handler
initialize();
glutMainLoop();	
	