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

var win = {};
win.width = 640;
win.height = 480;
win.title = "OpenGL/GLUT Example 05";
win.field_of_view_angle = 45;
win.z_near = 1.0f;
win.z_far = 500.0f;

print win

var RANGE = 150
var starList = []

for (var i = 0; i < 1000; ++i){
	starList[] = { 
		x = math.random(-RANGE, RANGE), 
		y = math.random(-RANGE, RANGE), 
		z = math.random(-500, -300), 
	}
}

var display_time = 0

function display(){
	var start_time = getTimeSec()
	
	glClear(GL_COLOR_BUFFER_BIT);
	for (var i, star in starList){
		var L = 0.3 + (star.z + 500) / 500 * 0.7;
		glPointSize(1.0f + 1.0f - (50 - star.z) / 550.0f);
		glColor3f(L * (0.5 + 0.5 * (i % 100) / 100), 
				  L * (0.5 + 0.5 * (i % 70) / 70), 
				  L * (0.5 + 0.5 * (i % 120) / 120));
		glBegin(GL_POINTS);
		glVertex3f(star.x, star.y, star.z);
		glEnd();
	}
	glutSwapBuffers();
	
	var cur_time = getTimeSec()
	var dt = cur_time - display_time
	display_time = cur_time
	// printf("[display] step dt: %.5f, dt: %.5f, fps: %.1f\n", cur_time - start_time, dt, 1/dt)
}

KEY_ESCAPE = 27

function keyboard(key, mousePositionX, mousePositionY){ 
	if(key == KEY_ESCAPE){        
		process.exit(0);
	}
}

var tick = 0
var time = 0

function timer(){	
	var start_time = getTimeSec()
	
	for (var _, star in starList){
		star.z = star.z + 0.6;
		if (star.z > 0){
			star.x = math.random(-RANGE, RANGE);
			star.y = math.random(-RANGE, RANGE);
			star.z = math.random(-500, -300);
		}	
	}
	
	glutPostRedisplay();
	glutTimerFunc(0, timer);
	
	var cur_time = getTimeSec()
	var dt = cur_time - time
	time = cur_time
	tick = tick + 1
	//  printf("step dt: %.5f, dt: %.5f, fps: %.1f, tick: %d\n", cur_time - start_time, dt, 1/dt, tick)
}

// initialize and run program
glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);  // Display Mode
glutInitWindowSize(win.width, win.height);					// set window size
glutCreateWindow(win.title);								// create Window

glClearColor(0, 0, 0, 1.0);

glMatrixMode(GL_PROJECTION);
glLoadIdentity();

var aspect = win.width / win.height;
gluPerspective(win.field_of_view_angle, aspect, win.z_near, win.z_far);	

glutDisplayFunc(display);
glutKeyboardFunc(keyboard);

timer();
glutMainLoop();
	