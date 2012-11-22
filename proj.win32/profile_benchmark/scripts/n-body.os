var start_time = os.clock()

var PI = 3.141592653589793
var SOLAR_MASS = 4 * PI * PI
var DAYS_PER_YEAR = 365.24
var bodies = {
  // simulate lua like table, lua starts from 1
  1: { // Sun
    x = 0,
    y = 0,
    z = 0,
    vx = 0,
    vy = 0,
    vz = 0,
    mass = SOLAR_MASS
  },
  { // Jupiter
    x = 4.84143144246472090e+00,
    y = -1.16032004402742839e+00,
    z = -1.03622044471123109e-01,
    vx = 1.66007664274403694e-03 * DAYS_PER_YEAR,
    vy = 7.69901118419740425e-03 * DAYS_PER_YEAR,
    vz = -6.90460016972063023e-05 * DAYS_PER_YEAR,
    mass = 9.54791938424326609e-04 * SOLAR_MASS
  },
  { // Saturn
    x = 8.34336671824457987e+00,
    y = 4.12479856412430479e+00,
    z = -4.03523417114321381e-01,
    vx = -2.76742510726862411e-03 * DAYS_PER_YEAR,
    vy = 4.99852801234917238e-03 * DAYS_PER_YEAR,
    vz = 2.30417297573763929e-05 * DAYS_PER_YEAR,
    mass = 2.85885980666130812e-04 * SOLAR_MASS
  },
  { // Uranus
    x = 1.28943695621391310e+01,
    y = -1.51111514016986312e+01,
    z = -2.23307578892655734e-01,
    vx = 2.96460137564761618e-03 * DAYS_PER_YEAR,
    vy = 2.37847173959480950e-03 * DAYS_PER_YEAR,
    vz = -2.96589568540237556e-05 * DAYS_PER_YEAR,
    mass = 4.36624404335156298e-05 * SOLAR_MASS
  },
  { // Neptune
    x = 1.53796971148509165e+01,
    y = -2.59193146099879641e+01,
    z = 1.79258772950371181e-01,
    vx = 2.68067772490389322e-03 * DAYS_PER_YEAR,
    vy = 1.62824170038242295e-03 * DAYS_PER_YEAR,
    vz = -9.51592254519715870e-05 * DAYS_PER_YEAR,
    mass = 5.15138902046611451e-05 * SOLAR_MASS
  }
}

var function advance(bodies, nbody, dt){
  for(var i=1; i<=nbody; i++){
    var bi = bodies[i]
    var bix, biy, biz, bimass = bi.x, bi.y, bi.z, bi.mass
    var bivx, bivy, bivz = bi.vx, bi.vy, bi.vz
    for(var j=i+1; j<=nbody; j++){
      var bj = bodies[j]
      var dx, dy, dz = bix-bj.x, biy-bj.y, biz-bj.z
      var mag = (dx*dx + dy*dy + dz*dz) ** 0.5
      mag = dt / (mag * mag * mag)
      var bm = bj.mass*mag
      bivx = bivx - (dx * bm)
      bivy = bivy - (dy * bm)
      bivz = bivz - (dz * bm)
      bm = bimass*mag
      bj.vx = bj.vx + (dx * bm)
      bj.vy = bj.vy + (dy * bm)
      bj.vz = bj.vz + (dz * bm)
    }
    bi.vx = bivx
    bi.vy = bivy
    bi.vz = bivz
    bi.x = bix + dt * bivx
    bi.y = biy + dt * bivy
    bi.z = biz + dt * bivz
  }
}

var function energy(bodies, nbody){
  var e = 0
  for(var i=1; i<=nbody; i++){
    var bi = bodies[i]
    var vx, vy, vz, bim = bi.vx, bi.vy, bi.vz, bi.mass
    e = e + (0.5 * bim * (vx*vx + vy*vy + vz*vz))
    for(var j=i+1; j<=nbody; j++){
      var bj = bodies[j]
      var dx, dy, dz = bi.x-bj.x, bi.y-bj.y, bi.z-bj.z
      var distance = (dx*dx + dy*dy + dz*dz) ** 0.5
      e = e - ((bim * bj.mass) / distance)
    }
  }
  return e
}

var function offsetMomentum(b, nbody){
  var px, py, pz = 0, 0, 0
  for(var i=1; i<=nbody; i++){
    var bi = b[i]
    var bim = bi.mass
    px = px + (bi.vx * bim)
    py = py + (bi.vy * bim)
    pz = pz + (bi.vz * bim)
  }
  b[1].vx = -px / SOLAR_MASS
  b[1].vy = -py / SOLAR_MASS
  b[1].vz = -pz / SOLAR_MASS
}

var N = numberOf(arg && arg[1]) || 1000
var nbody = #bodies

offsetMomentum(bodies, nbody)
printf("%0.9f\n", energy(bodies, nbody))
// debugger
for(var i=1; i<=N; i++){ advance(bodies, nbody, 0.01) }
printf("%0.9f\n",energy(bodies, nbody))

printf("time: %.3f\n", getTimeSec() - start_time)