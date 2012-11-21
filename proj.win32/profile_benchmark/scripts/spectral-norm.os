var start_time = getTimeSec()

var function A(i, j){
  var ij = i+j-1
  return 1.0 / (ij * (ij-1) * 0.5 + i)
}

var function Av(x, y, N){
  for(var i=1; i <= N; i++){
    var a = 0
    for(var j=1; j <= N; j++){ a = a + x[j] * A(i, j) }
    y[i] = a
  }
}

var function Atv(x, y, N){
  for(var i=1; i<=N; i++){
    var a = 0
    for(var j=1; j<=N; j++){ a = a + x[j] * A(j, i) }
    y[i] = a
  }
}

var function AtAv(x, y, t, N){
  Av(x, t, N)
  Atv(t, y, N)
}

var N = numberOf(arg && arg[1]) || 100
var u, v, t = {}, {}, {}
for(var i=1; i<=N; i++){ u[i] = 1 }

for(var i=1; i<=10; i++){ AtAv(u, v, t, N) AtAv(v, u, t, N) }

var vBv, vv = 0, 0
for(var i=1; i<=N; i++){
  var ui, vi = u[i], v[i]
  vBv = vBv + ui*vi
  vv = vv + vi*vi
}
printf("%0.9f\n", math.sqrt(vBv / vv))
printf("time: %.3f\n", getTimeSec() - start_time)