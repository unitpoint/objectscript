print "getcwd: "..getcwd()

var t = TestClass(1, 0.25)
print "t.i: "..t.i
print "t.j: "..t.j

print "t.doSomething(10, 100.001, 1000.1): "..t.doSomething(10, 100.001, 1000.1)

var t2 = clone t
t2.i = t2.i + 10
t2.j = t2.j + 20.5

var t3 = t + t2
t3.print()

print "isdigit(123): "..my.isdigit("123")
print "isdigit(123q): "..my.isdigit("123q")

print "my.hash(123): "..my.hash(123)
/* Note: it should be converted to int due to\n'void my_print_num(int i)' declaration */
print "call my.print_num(123.5)"
my.print_num(123.5)
print "call my.print_void()"
my.print_void()
print "my.abs(-12): "..my.abs(-12)
print "my.fabs(-123.5): "..my.fabs(-123.5)