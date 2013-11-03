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
win.title = "OpenGL/GLUT Example 04";
win.field_of_view_angle = 45;
win.z_near = 1.0f;
win.z_far = 500.0f;

var W = math.floor(win.width / 9)
var H = math.floor(win.height / 15)

win.width = W * 9
win.height = H * 15

var MAX_LIVE = 100

print win

function getRandChar(){
	return 33 + math.random(256-33);
}

var screen = []
for (var x = 0; x < W; ++x){
	screen[x] = []
	for (var y = 0; y < H; ++y){
		screen[x][y] = {
			live = MAX_LIVE * 0.5 * y / H,
			ch = getRandChar(),
		}
	}
}

var display_time = 0

function display(){
	var start_time = getTimeSec()
	
	glClear(GL_COLOR_BUFFER_BIT);
	for (var x = 0; x < W; x = x + 2){
		var col = screen[x]
		for (var y = 0; y < H; ++y){
			var pos = col[y]
			if(pos.live > 0){
				glRasterPos2f(x * 9, (y + 1) * 15);
				glColor3f(0, pos.live / MAX_LIVE, 0);
				glutBitmapCharacter(GLUT_BITMAP_9_BY_15, pos.ch);
			}
		}
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
	
	for (var y = 0; y < H; ++y)
		for (var x = 0; x < W; x = x + 2){
			var pos = screen[x][y]
			if (pos.live > 0)
				pos.live = pos.live - 1 // math.random(1, 2);
		}
		
	for(var i = 0; i < W*0.1; i++){
		var x = math.random(W/2) * 2;
		for (var y = 0; y < H; ++y){
			var pos = screen[x][y]
			if (pos.live <= 0){
				pos.ch = getRandChar();
				pos.live = MAX_LIVE;
				break;
			}
		}
	}
	// if(false)
	for(var i = 0; i < W*0.03; i++){
		var x = math.random(W/2) * 2;
		for (var y = H-1; y > 0; --y){
			screen[x][y] = screen[x][y-1]
		}
		screen[x][0] = {
			ch = 32 + getRandChar(),
			live = 0, // MAX_LIVE,
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
glOrtho(0, win.width, win.height, 0, -1, 1);
glutDisplayFunc(display);
glutKeyboardFunc(keyboard);

timer();
glutMainLoop();
	