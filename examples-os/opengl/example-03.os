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
win.title = "OpenGL/GLUT Example 03";
win.field_of_view_angle = 45;
win.z_near = 1.0f;
win.z_far = 500.0f;

print win

var gl_rotation = 0;

function display(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		     // Clear Screen and Depth Buffer
	glLoadIdentity();
	gluLookAt( 10,3,0, 0,0,0, 0,1,0);					  // Define a viewing transformation

	glPushMatrix();										  // Push the current matrix stack
		glColor3f(1,0,0);
		glTranslatef(0,0,-2);							  // Multiply the current matrix by a translation matrix
		glRotatef(gl_rotation,0,1,0);						  // Multiply the current matrix by a rotation matrix 
		glRotatef(90,0,1,0);							  // Multiply the current matrix by a rotation matrix
	    glutWireTeapot(1);								  // render a wire­frame teapot respectively. 
	glPopMatrix();										  // Pop the current matrix stack

	glPushMatrix();										  // Push the current matrix stack	
		glColor3f(0,1,0);
		glTranslatef(0,0,2);							  // Multiply the current matrix by a translation matrix
		glRotatef(gl_rotation,0,1,0);
		glRotatef(90,0,1,0);
	    glutSolidTeapot(1);
	glPopMatrix();										  // Pop the current matrix stack

	glPushMatrix();										  // Push the current matrix stack
		glColor3f(0,0,1);
		glRotatef(-gl_rotation,0,1,0);
		glRotatef(90,0,1,0);							  // Multiply the current matrix by a rotation matrix 
		glTranslatef(0,2,0);							  // Multiply the current matrix by a translation matrix
		glutSolidCube  (1.3); 
	glPopMatrix();										  // Pop the current matrix stack

	glPushMatrix();									      // Push the current matrix stack
		glColor3f(1,1,1);
		glTranslatef(0,-2.5,0);							  // Multiply the current matrix by a translation matrix
		glRotatef(-gl_rotation,1,1,0);					 	  // Multiply the current matrix by a rotation matrix 
		glRotatef(90,0,1,0);							  // Multiply the current matrix by a rotation matrix 
		glutSolidSphere  (1 , 32 , 32 ); 	
	glPopMatrix();									      // Pop the current matrix stack
	
	gl_rotation = gl_rotation + 1;

	glutSwapBuffers();
}

function initialize(){
    glMatrixMode(GL_PROJECTION);												// select projection matrix
    glViewport(0, 0, win.width, win.height);									// set the viewport
    glMatrixMode(GL_PROJECTION);												// set matrix mode
    glLoadIdentity();															// reset projection matrix
    var aspect = win.width / win.height;
	gluPerspective(win.field_of_view_angle, aspect, win.z_near, win.z_far);		// set up a perspective projection matrix
    glMatrixMode(GL_MODELVIEW);													// specify which matrix is the current matrix
    glShadeModel( GL_SMOOTH );
    glClearDepth( 1.0f );														// specify the clear value for the depth buffer
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );						// specify implementation-specific hints

	var amb_light = { 0.1, 0.1, 0.1, 1.0 };
    var diffuse = { 0.6, 0.6, 0.6, 1 };
    var specular = { 0.7, 0.7, 0.3, 1 };
    glLightModelfv( GL_LIGHT_MODEL_AMBIENT, amb_light );
    glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuse );
    glLightfv( GL_LIGHT0, GL_SPECULAR, specular );
    glEnable( GL_LIGHT0 );
    glEnable( GL_COLOR_MATERIAL );
    glShadeModel( GL_SMOOTH );
    glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE );
    glDepthFunc( GL_LEQUAL );
    glEnable( GL_DEPTH_TEST );
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0); 
	glClearColor(0.0, 0.0, 1.0, 1.0);
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
	