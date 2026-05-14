/*
 DriveNet - ESP32: Web UI + MPU6050 + HC-SR04 + Servo + ESP-NOW
 - Web UI (SoftAP) serves raw local data and neighbor raw packets.
 - ESP-NOW broadcasts local raw packet and receives neighbors.
 - Client (browser) does all calculations (lane classification, warnings, merge).
 - Change CAR_ID for each car.
*/

// ======= Includes =======
#include <WiFi.h>
#include <WebServer.h>
#include <esp_now.h>
#include <Wire.h>
#include <MPU6050_light.h>
#include <ESP32Servo.h>

// ======= Config =======
#define CAR_ID 1                           // set unique ID for this car (1..254)
const char *AP_SSID = "DriveNet_AP";      // softAP SSID
const char *AP_PASS = "drivenet123";      // softAP password

// Pins (adjust if you re-wired)
const int SDA_PIN = 21;    // MPU6050 SDA
const int SCL_PIN = 22;    // MPU6050 SCL
const int TRIG_PIN = 32;   // HC-SR04 TRIG
const int ECHO_PIN = 33;   // HC-SR04 ECHO (use voltage divider)
const int SERVO_PIN = 27;  // Servo signal

// Sensor/sample timing
const unsigned long SAMPLE_INTERVAL_MS = 150;
const unsigned long ESPNOW_SEND_INTERVAL_MS = 500;
const unsigned long SWEEP_INTERVAL_MS = 120;
const unsigned long NEIGHBOR_TTL_MS = 2500; // expire neighbor entries after 2.5s

// ======= Global objects =======
WebServer server(80);
MPU6050 mpu(Wire);
Servo myServo;

// raw local values
volatile float ax=0, ay=0, az=0;
volatile float gx=0, gy=0, gz=0;
volatile float distanceCM = -1.0f;
volatile int currentServoAngle = 90;
volatile bool sweepEnabled = false;

// timing
unsigned long lastSampleMs = 0;
unsigned long lastEspNowSendMs = 0;
unsigned long lastSweepMs = 0;

// ======= Ultrasonic constants =======
const unsigned long ULTRA_TIMEOUT_US = 30000UL; // 30 ms
const float SPEED_SOUND_CM_US = 0.0343f;       // cm per us

// ======= ESP-NOW packet (packed, scaled) =======
#pragma pack(push,1)
typedef struct {
  uint8_t carId;        // 1..254
  uint16_t seq;         // incrementing seq number
  int16_t ax;           // accel *1000 (g -> milli-g)
  int16_t ay;
  int16_t az;
  int16_t gx;           // gyro *100 (deg/s -> centi-degree/s)
  int16_t gy;
  int16_t gz;
  int16_t distance;     // distance_cm *100 (centi-cm), 32767 => no reading
  uint8_t servoAngle;   // 0..180
  uint32_t tstamp_ms;   // sender millis()
} DriveNetPkt;
#pragma pack(pop)

uint16_t sendSeq = 0;
uint8_t broadcastMac[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

// ======= Neighbor store =======
// Simple fixed-size store keyed by carId (0 unused). Stores last packet and last seen time.
struct Neighbor {
  bool valid;
  DriveNetPkt pkt;
  uint32_t lastSeenMs;
};
const int MAX_NEIGHBORS = 16;
Neighbor neighbors[MAX_NEIGHBORS];

// ======= Helper functions =======
float readUltrasonicCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  unsigned long dur = pulseIn(ECHO_PIN, HIGH, ULTRA_TIMEOUT_US);
  if (dur == 0UL) return -1.0f;
  float d = (dur * SPEED_SOUND_CM_US) / 2.0f;
  return d;
}

int findNeighborIndexByCarId(uint8_t id) {
  for (int i=0;i<MAX_NEIGHBORS;i++) if (neighbors[i].valid && neighbors[i].pkt.carId == id) return i;
  return -1;
}
int findFreeNeighborIndex() {
  for (int i=0;i<MAX_NEIGHBORS;i++) if (!neighbors[i].valid) return i;
  return -1;
}
void storeNeighbor(const DriveNetPkt &p) {
  int idx = findNeighborIndexByCarId(p.carId);
  if (idx < 0) {
    idx = findFreeNeighborIndex();
    if (idx < 0) {
      // overwrite oldest
      uint32_t oldest = UINT32_MAX; int oi=0;
      for (int i=0;i<MAX_NEIGHBORS;i++) {
        if (neighbors[i].lastSeenMs < oldest) { oldest = neighbors[i].lastSeenMs; oi=i; }
      }
      idx = oi;
    }
  }
  neighbors[idx].valid = true;
  neighbors[idx].pkt = p;
  neighbors[idx].lastSeenMs = millis();
}

// ======= ESP-NOW callbacks (newer signatures) =======
void onDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  (void)info; (void)status;
  // optional debug: Serial.printf("[ESP-NOW] send status=%d\n",(int)status);
}

void onDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
  if (!data || len < (int)sizeof(DriveNetPkt)) return;
  DriveNetPkt r;
  memcpy(&r, data, sizeof(r));

  // store neighbor (use carId) and print to Serial for debug
  r.tstamp_ms = r.tstamp_ms; // already set by sender
  storeNeighbor(r);

  // optional debug print (short)
  char macs[18]; snprintf(macs,sizeof(macs), "%02X:%02X:%02X:%02X:%02X:%02X",
      recv_info->src_addr[0],recv_info->src_addr[1],recv_info->src_addr[2],
      recv_info->src_addr[3],recv_info->src_addr[4],recv_info->src_addr[5]);
  float rax = r.ax/1000.0f, ray = r.ay/1000.0f, raz = r.az/1000.0f;
  float rgx = r.gx/100.0f, rgy = r.gy/100.0f, rgz = r.gz/100.0f;
  float rdist = (r.distance==32767 ? -1.0f : r.distance/100.0f);
  Serial.printf("[ESP-NOW RX] from %s car=%u seq=%u t=%u ms\n", macs, r.carId, r.seq, r.tstamp_ms);
  Serial.printf("  Accel(g)=%.3f %.3f %.3f  Gyro=%.2f %.2f %.2f  Dist=%.2fcm  Servo=%u\n",
                rax,ray,raz, rgx,rgy,rgz, rdist, r.servoAngle);
}

// ======= ESP-NOW send (pack and broadcast) =======
void sendEspNowPacket() {
  DriveNetPkt p;
  p.carId = (uint8_t)CAR_ID;
  p.seq = sendSeq++;
  p.ax = (int16_t)round(ax * 1000.0f);
  p.ay = (int16_t)round(ay * 1000.0f);
  p.az = (int16_t)round(az * 1000.0f);
  p.gx = (int16_t)round(gx * 100.0f);
  p.gy = (int16_t)round(gy * 100.0f);
  p.gz = (int16_t)round(gz * 100.0f);
  if (distanceCM < 0) p.distance = 32767; else p.distance = (int16_t)round(distanceCM * 100.0f);
  p.servoAngle = (uint8_t)currentServoAngle;
  p.tstamp_ms = millis();

  esp_err_t r = esp_now_send(broadcastMac, (uint8_t*)&p, sizeof(p));
  if (r != ESP_OK) {
    Serial.printf("[ESP-NOW] send error %d\n", r);
  }
}

// ======= Web UI (single page). Client will do all calculations. =======
const char index_html[] PROGMEM = R"rawliteral(
<!doctype html><html><head>
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>DriveNet Raw UI</title>
<style>
 body{font-family:Inter,system-ui,Arial;padding:12px;background:#f5f7fb;color:#111}
 h1{margin:0 0 8px 0}
 .card{background:#fff;border-radius:8px;padding:10px;margin-bottom:10px;box-shadow:0 1px 6px rgba(0,0,0,0.06)}
 .row{display:flex;gap:8px;flex-wrap:wrap}
 .cell{flex:1 1 140px}
 .big{font-weight:700;font-size:20px}
 button{padding:8px 10px;border-radius:6px;border:1px solid #ccc;background:#fff}
 table{width:100%;border-collapse:collapse}
 th,td{padding:6px;border-bottom:1px solid #eee}
 .warn{color:#b71c1c;font-weight:700}
</style>
</head><body>
<h1>DriveNet — Raw Sensor UI</h1>

<div class="card">
 <div style="display:flex;justify-content:space-between;align-items:center">
  <div>
    <div><b>Device</b>: Car <span id="carid">?</span></div>
    <div id="ipinfo" style="font-size:12px;color:#666"></div>
  </div>
  <div>
    <input id="angleInput" type="number" min="0" max="180" value="90" style="width:80px">
    <button onclick="setAngle()">Set Angle</button>
    <button onclick="toggleSweep()"><span id="sweepBtn">Start Sweep</span></button>
  </div>
 </div>
</div>

<div class="card row">
 <div class="cell"><div>Accel X (g)</div><div id="ax" class="big">--</div></div>
 <div class="cell"><div>Accel Y (g)</div><div id="ay" class="big">--</div></div>
 <div class="cell"><div>Accel Z (g)</div><div id="az" class="big">--</div></div>
 <div class="cell"><div>Gyro X (deg/s)</div><div id="gx" class="big">--</div></div>
 <div class="cell"><div>Gyro Y (deg/s)</div><div id="gy" class="big">--</div></div>
 <div class="cell"><div>Gyro Z (deg/s)</div><div id="gz" class="big">--</div></div>
</div>

<div class="card row">
 <div class="cell"><div>Distance</div><div id="distance" class="big">-- cm</div></div>
 <div class="cell"><div>Servo Angle</div><div id="sangle" class="big">--°</div></div>
 <div class="cell"><div>Last sample</div><div id="last" class="big">--</div></div>
</div>

<div class="card">
 <h3>Nearby Cars (raw)</h3>
 <div id="neighborsDiv">Loading...</div>
</div>

<script>
const carId = %CAR_ID%;
document.getElementById('carid').innerText = carId;
document.getElementById('ipinfo').innerText = 'Connect to Wi-Fi: DriveNet_AP / drivenet123';

let sweep = false;
function setAngle(){
  const v = Number(document.getElementById('angleInput').value);
  if (isNaN(v) || v<0 || v>180){ alert('0-180'); return; }
  fetch('/servo?angle=' + v).then(()=>{});
}
function toggleSweep(){
  sweep = !sweep;
  fetch('/sweep?enable=' + (sweep?1:0)).then(()=>{
    document.getElementById('sweepBtn').innerText = sweep ? 'Stop Sweep' : 'Start Sweep';
  });
}
async function poll(){
  try{
    const r = await fetch('/rawdata'); const j = await r.json();
    document.getElementById('ax').innerText = j.ax.toFixed(3);
    document.getElementById('ay').innerText = j.ay.toFixed(3);
    document.getElementById('az').innerText = j.az.toFixed(3);
    document.getElementById('gx').innerText = j.gx.toFixed(2);
    document.getElementById('gy').innerText = j.gy.toFixed(2);
    document.getElementById('gz').innerText = j.gz.toFixed(2);
    document.getElementById('distance').innerText = (j.distance_cm<0)?'--':j.distance_cm.toFixed(2)+' cm';
    document.getElementById('sangle').innerText = j.servo_angle + '°';
    document.getElementById('last').innerText = new Date(j.tstamp).toLocaleTimeString();
  }catch(e){ console.log('rawdata err',e); }

  try{
    const r2 = await fetch('/neighbors'); const nb = await r2.json();
    if (nb.length === 0) document.getElementById('neighborsDiv').innerText = 'No neighbors';
    else {
      let html = '<table><tr><th>Car</th><th>LastSeen</th><th>Accel g</th><th>Gyro</th><th>Dist cm</th><th>Servo°</th></tr>';
      for (const p of nb) {
        html += `<tr>
          <td>${p.carId}</td>
          <td>${new Date(p.tstamp).toLocaleTimeString()}</td>
          <td>${p.ax.toFixed(3)} ${p.ay.toFixed(3)} ${p.az.toFixed(3)}</td>
          <td>${p.gx.toFixed(2)} ${p.gy.toFixed(2)} ${p.gz.toFixed(2)}</td>
          <td>${p.distance_cm<0?'--':p.distance_cm.toFixed(2)}</td>
          <td>${p.servoAngle}</td>
        </tr>`;
      }
      html += '</table>';
      document.getElementById('neighborsDiv').innerHTML = html;
    }
  }catch(e){ console.log('neighbors err', e); }

  setTimeout(poll, 300);
}
poll();
</script>
</body></html>
)rawliteral";

// ======= Web handlers =======
void handleRoot() { 
  String html = String(index_html);
  html.replace("%CAR_ID%", String(CAR_ID));
  server.send(200, "text/html", html);
}

void handleRawData() {
  // prepare JSON of local sensors
  String json = "{";
  json += "\"carId\":" + String(CAR_ID) + ",";
  json += "\"ax\":" + String(ax, 3) + ",";
  json += "\"ay\":" + String(ay, 3) + ",";
  json += "\"az\":" + String(az, 3) + ",";
  json += "\"gx\":" + String(gx, 2) + ",";
  json += "\"gy\":" + String(gy, 2) + ",";
  json += "\"gz\":" + String(gz, 2) + ",";
  json += "\"distance_cm\":" + String(distanceCM, 2) + ",";
  json += "\"servo_angle\":" + String(currentServoAngle) + ",";
  json += "\"tstamp\":" + String(millis());
  json += "}";
  server.send(200, "application/json", json);
}

void handleNeighbors() {
  // return array of neighbor pkt JSON (only fresh ones)
  String json = "[";
  uint32_t now = millis();
  bool first = true;
  for (int i=0;i<MAX_NEIGHBORS;i++) {
    if (!neighbors[i].valid) continue;
    if ((uint32_t)(now - neighbors[i].lastSeenMs) > NEIGHBOR_TTL_MS) { neighbors[i].valid = false; continue; }
    DriveNetPkt &p = neighbors[i].pkt;
    if (!first) json += ",";
    first = false;
    float pax = p.ax/1000.0f, pay = p.ay/1000.0f, paz = p.az/1000.0f;
    float pgx = p.gx/100.0f, pgy = p.gy/100.0f, pgz = p.gz/100.0f;
    float pd = (p.distance==32767 ? -1.0f : p.distance/100.0f);
    json += "{";
    json += "\"carId\":" + String(p.carId) + ",";
    json += "\"ax\":" + String(pax,3) + ",";
    json += "\"ay\":" + String(pay,3) + ",";
    json += "\"az\":" + String(paz,3) + ",";
    json += "\"gx\":" + String(pgx,2) + ",";
    json += "\"gy\":" + String(pgy,2) + ",";
    json += "\"gz\":" + String(pgz,2) + ",";
    json += "\"distance_cm\":" + String(pd,2) + ",";
    json += "\"servoAngle\":" + String(p.servoAngle) + ",";
    json += "\"tstamp\":" + String(p.tstamp_ms);
    json += "}";
  }
  json += "]";
  server.send(200, "application/json", json);
}

void handleServo() {
  if (!server.hasArg("angle")) { server.send(400, "text/plain", "missing angle"); return; }
  int a = server.arg("angle").toInt();
  a = constrain(a, 0, 180);
  currentServoAngle = a;
  myServo.write(a);
  server.send(200, "text/plain", "OK");
}

void handleSweep() {
  if (!server.hasArg("enable")) { server.send(400, "text/plain", "missing"); return; }
  sweepEnabled = server.arg("enable").toInt() != 0;
  server.send(200, "text/plain", sweepEnabled ? "on" : "off");
}

// ======= Initialize ESP-NOW (and SoftAP) =======
void initEspNow() {
  WiFi.mode(WIFI_AP_STA); // allow SoftAP + ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("esp_now_init failed");
    return;
  }
  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(onDataRecv);

  // add broadcast peer (some cores require peer addition)
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, broadcastMac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_err_t res = esp_now_add_peer(&peerInfo);
  (void)res;
}

// ======= Setup =======
void setup() {
  Serial.begin(115200);
  delay(20);
  Serial.println("DriveNet ESP32 starting...");

  // I2C and MPU
  Wire.begin(SDA_PIN, SCL_PIN);
  byte s = mpu.begin();
  Serial.printf("MPU init status: %u\n", s);
  Serial.println("Calibrating gyro - keep device still...");
  delay(500);
  mpu.calcGyroOffsets();
  Serial.println("MPU ready.");

  // ultrasonic
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  // servo
  myServo.attach(SERVO_PIN);
  currentServoAngle = 90;
  myServo.write(currentServoAngle);
  delay(100);

  // SoftAP
  WiFi.softAP(AP_SSID, AP_PASS);
  IPAddress ip = WiFi.softAPIP();
  Serial.print("AP IP: "); Serial.println(ip);

  // ESP-NOW
  initEspNow();

  // web routes
  server.on("/", handleRoot);
  server.on("/rawdata", handleRawData);
  server.on("/neighbors", handleNeighbors);
  server.on("/servo", handleServo);
  server.on("/sweep", handleSweep);
  server.begin();

  // clear neighbor store
  for (int i=0;i<MAX_NEIGHBORS;i++) neighbors[i].valid = false;

  lastSampleMs = millis();
  lastEspNowSendMs = millis();
  lastSweepMs = millis();
}

// ======= Main loop =======
void loop() {
  server.handleClient();
  unsigned long now = millis();

  // periodic sample
  if (now - lastSampleMs >= SAMPLE_INTERVAL_MS) {
    lastSampleMs = now;
    mpu.update();
    ax = mpu.getAccX();
    ay = mpu.getAccY();
    az = mpu.getAccZ();
    gx = mpu.getGyroX();
    gy = mpu.getGyroY();
    gz = mpu.getGyroZ();
    distanceCM = readUltrasonicCm();
  }

  // servo sweep logic
  if (sweepEnabled && (now - lastSweepMs >= SWEEP_INTERVAL_MS)) {
    lastSweepMs = now;
    currentServoAngle += 3;
    if (currentServoAngle > 180) currentServoAngle = 0;
    myServo.write(currentServoAngle);
  }

  // ESP-NOW periodic send
  if (now - lastEspNowSendMs >= ESPNOW_SEND_INTERVAL_MS) {
    lastEspNowSendMs = now;
    sendEspNowPacket();
  }

  // expire neighbors older than TTL
  for (int i=0;i<MAX_NEIGHBORS;i++) {
    if (neighbors[i].valid && (millis() - neighbors[i].lastSeenMs) > NEIGHBOR_TTL_MS) neighbors[i].valid = false;
  }

  delay(1); // let background tasks run
}
