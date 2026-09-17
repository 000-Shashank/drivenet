const randomBetween = (min, max) => Math.random() * (max - min) + min;

const ax = document.getElementById('ax');
const gz = document.getElementById('gz');
const distance = document.getElementById('distance');
const neighborCount = document.getElementById('neighborCount');

function tickDemo() {
  const axVal = randomBetween(0.1, 0.9).toFixed(3);
  const gzVal = randomBetween(5, 35).toFixed(2);
  const distanceVal = randomBetween(20, 120).toFixed(1);

  ax.textContent = `${axVal}g`;
  gz.textContent = `${gzVal}°`;
  distance.textContent = `${distanceVal} cm`;
  neighborCount.textContent = String(Math.floor(randomBetween(2, 5)));
}

setInterval(tickDemo, 1200);

tickDemo();

// If the user later hosts the real ESP32 dashboard, this is the placeholder area that can be replaced.
// The actual project is served directly by the ESP32 at http://192.168.4.1.
