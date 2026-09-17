const randomBetween = (min, max) => Math.random() * (max - min) + min;

const ax = document.getElementById('ax');
const gz = document.getElementById('gz');
const distance = document.getElementById('distance');
const neighborCount = document.getElementById('neighborCount');

function tickDemo() {
  ax.textContent = `${randomBetween(0.1, 0.9).toFixed(3)}g`;
  gz.textContent = `${randomBetween(5, 35).toFixed(2)}°`;
  distance.textContent = `${randomBetween(20, 120).toFixed(1)} cm`;
  neighborCount.textContent = String(Math.floor(randomBetween(2, 5)));
}

setInterval(tickDemo, 1200);
tickDemo();
