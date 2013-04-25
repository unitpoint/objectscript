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
			t.object && t.object !== _G ? "<${typeOf(t.object)}#${t.object.id}>.${t.name}" : t.name)
	}
}

win = {};
win.width = 640;
win.height = 480;
win.title = "OpenGL/GLUT Example 02";
win.field_of_view_angle = 45;
win.z_near = 1.0f;
win.z_far = 500.0f;

print win

var g_rotation = 0;
var g_rotation_speed = 0.2f;

function display(){
	// Clear Screen and Depth Buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		     
	glLoadIdentity();
 
	// Define a viewing transformation
	gluLookAt( 4,2,0, 0,0,0, 0,1,0);					  
 
 
	// Push and pop the current matrix stack. 
	// This causes that translations and rotations on this matrix wont influence others.
 
	glPushMatrix();										
		glColor3f(1,0,0);
		glTranslatef(0,0,0);							
		glRotatef(g_rotation,0,1,0);
		glRotatef(90,0,1,0);

		// Draw the teapot
	    glutSolidTeapot(1);
	glPopMatrix();										  
 
 
	g_rotation = g_rotation + g_rotation_speed;
	glutSwapBuffers();
}

function initialize(){
	// select projection matrix
	glMatrixMode(GL_PROJECTION);												

	// set the viewport
	glViewport(0, 0, win.width, win.height);									

	// set matrix mode
	glMatrixMode(GL_PROJECTION);												

	// reset projection matrix
	glLoadIdentity();															
	var aspect = win.width / win.height;

	// set up a perspective projection matrix
	gluPerspective(win.field_of_view_angle, aspect, win.z_near, win.z_far);		

	// specify which matrix is the current matrix
	glMatrixMode(GL_MODELVIEW);													
	glShadeModel( GL_SMOOTH );

	// specify the clear value for the depth buffer
	glClearDepth( 1.0f );														
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LEQUAL );

	// specify implementation-specific hints
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );						

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
	glClearColor(0.0, 0.0, 0.0, 1.0);
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
	