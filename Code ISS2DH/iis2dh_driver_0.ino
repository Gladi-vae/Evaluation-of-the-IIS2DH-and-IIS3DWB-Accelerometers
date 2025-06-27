// -------------------------------------------------------------------------------------------------
//
// Auteur: Victor Fourel, Date: Juin 2025
//
// Ce code permet de créer un wifi sur un esp32 Dev Kit C-1. Une fois connecté avec un pc, on peut accéder à la page 
// web 192.168.1 qui permet de configurer le capteur et de lancer l'acquisition des données.
// Le fichier CSV se télécharge automatiquement dans le navigateur au bout d'un moment.
//
//--------------------------------------------------------------------------------------------------
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "iis2dh.h"
#include <esp_heap_caps.h>  // Pour heap_caps_malloc()

#define MAX_SAMPLES 85000

WebServer server(80);

bool acquiring = false;
uint32_t start_time = 0;
size_t sample_count = 0;
uint32_t acq_duration_ms = 60000;  // Durée modifiable depuis la page web

struct Sample {
  uint32_t timestamp_us;
  float x, y, z;
};

Sample* samples = nullptr;

IIS2DH_Config current_config;
 

void start_acquisition() {
  if (samples) free(samples);
  samples = (Sample*) heap_caps_malloc(MAX_SAMPLES * sizeof(Sample), MALLOC_CAP_SPIRAM);
  if (!samples) {
    Serial.println("❌ Échec de l'allocation en PSRAM !");
    while (true);
  }

  sample_count = 0;
  acquiring = true;
  start_time = micros();
  Serial.printf("📈 Acquisition démarrée (%d échantillons max)\n", MAX_SAMPLES);
}

void stop_acquisition() {
  acquiring = false;
  Serial.println("🛑 Acquisition terminée");
}

String generateCSVHeader() {
  if (sample_count < 2) return "Pas assez de données\n";

  uint32_t min_diff = UINT32_MAX;
  uint32_t max_diff = 0;
  uint64_t sum_diff = 0;

  for (size_t i = 2; i < sample_count; i++) {
    uint32_t diff = samples[i].timestamp_us - samples[i - 1].timestamp_us;
    if (diff < min_diff) min_diff = diff;
    if (diff > max_diff) max_diff = diff;
    sum_diff += diff;
  }

  float avg_diff = sum_diff / float(sample_count - 2);
  float freq_avg = 1e6f / avg_diff;
  float freq_min = 1e6f / max_diff;
  float freq_max = 1e6f / min_diff;

  float freq_theorique = 0;
  switch (current_config.odr) {
    case IIS2DH_ODR_1HZ: freq_theorique = 1.0f; break;
    case IIS2DH_ODR_10HZ: freq_theorique = 10.0f; break;
    case IIS2DH_ODR_25HZ: freq_theorique = 25.0f; break;
    case IIS2DH_ODR_50HZ: freq_theorique = 50.0f; break;
    case IIS2DH_ODR_100HZ: freq_theorique = 100.0f; break;
    case IIS2DH_ODR_200HZ: freq_theorique = 200.0f; break;
    case IIS2DH_ODR_400HZ: freq_theorique = 400.0f; break;
    case IIS2DH_ODR_1344HZ: freq_theorique = 1344.0f; break;
    default: freq_theorique = 0;
  }

  String csv;
  csv += "=== Résumé acquisition ===\n";
  csv += "Fréquence théorique (Hz)," + String(freq_theorique, 2) + "\n";
  csv += "Écart moyen (µs)," + String(avg_diff, 2) + "\n";
  csv += "Écart min (µs)," + String(min_diff) + "\n";
  csv += "Écart max (µs)," + String(max_diff) + "\n";
  csv += "Fréquence moyenne (Hz)," + String(freq_avg, 2) + "\n";
  csv += "Fréquence min (Hz)," + String(freq_min, 2) + "\n";
  csv += "Fréquence max (Hz)," + String(freq_max, 2) + "\n\n";
  csv += "t(us),X(mg),Y(mg),Z(mg)\n";
  return csv;
}

void handleRoot() {
  String page = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head><title>IIS2DH Acquisition</title></head>
    <body>
      <h2>📊 Configuration Acquisition IIS2DH</h2>
      <form action="/start" method="get">
        <label>Mode:</label>
        <select name="mode">
          <option value="0">Low Power</option>
          <option value="1">Normal</option>
          <option value="2" selected>High Res</option>
        </select><br><br>

        <label>Échelle:</label>
        <select name="scale">
          <option value="2" selected>±2g</option>
          <option value="4">±4g</option>
          <option value="8">±8g</option>
          <option value="16">±16g</option>
        </select><br><br>

        <label>Fréquence (ODR):</label>
        <select name="odr">
          <option value="1">1 Hz</option>
          <option value="2">10 Hz</option>
          <option value="3">25 Hz</option>
          <option value="4">50 Hz</option>
          <option value="5">100 Hz</option>
          <option value="6">200 Hz</option>
          <option value="7">400 Hz</option>
          <option value="9" selected>1344 Hz</option>
        </select><br><br>

        <label>Durée (secondes) :</label>
        <input type="number" name="duration" min="1" max="60" value="20"><br><br>

        <input type="submit" value="Démarrer acquisition">
      </form>
    </body>
    </html>
  )rawliteral";
  server.send(200, "text/html", page);
}

void handleStart() {
  IIS2DH_Mode mode = (IIS2DH_Mode)server.arg("mode").toInt();
  IIS2DH_Scale scale = (IIS2DH_Scale)server.arg("scale").toInt();
  IIS2DH_ODR odr = (IIS2DH_ODR)server.arg("odr").toInt();
  int duration_s = server.arg("duration").toInt();

  if (duration_s < 1) duration_s = 1;
  if (duration_s > 60) duration_s = 60;

  acq_duration_ms = duration_s * 1000;

  current_config = {
    .mode = mode,
    .odr = odr,
    .scale = scale,
    .axes = IIS2DH_AXES_XYZ
  };

  iis2dh_init(current_config);
  delay(10);

  start_acquisition();

  String page = "<html><body><p>Acquisition en cours pendant " + String(duration_s) + "s... Veuillez patienter.</p><script>setTimeout(() => { window.location.href = \"/download\"; }, " + String((duration_s + 1) * 1000) + ");</script></body></html>";
  server.send(200, "text/html", page);
}

void handleDownload() {
  if (!samples || sample_count < 2) {
    server.send(200, "text/plain", "Pas assez de données acquises.");
    return;
  }

  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/csv");

  server.sendContent(generateCSVHeader());

  const size_t batch_size = 100;
  const size_t line_max_len = 64;
  const size_t buffer_size = batch_size * line_max_len + 1;

  char* buffer = (char*) malloc(buffer_size);
  if (!buffer) {
    server.sendContent("Erreur d'allocation mémoire.\n");
    return;
  }

  size_t buffer_index = 0;

  for (size_t i = 2; i < sample_count; ++i) {
    char line[line_max_len];
    int len = snprintf(line, sizeof(line), "%lu,%.2f,%.2f,%.2f\n",
                       samples[i].timestamp_us,
                       samples[i].x,
                       samples[i].y,
                       samples[i].z);

    if (buffer_index + len >= buffer_size - 1) {
      buffer[buffer_index] = '\0';
      server.sendContent(buffer);
      buffer_index = 0;
    }

    memcpy(buffer + buffer_index, line, len);
    buffer_index += len;
  }

  if (buffer_index > 0) {
    buffer[buffer_index] = '\0';
    server.sendContent(buffer);
  }

  free(buffer);
  Serial.println("✅ Fichier CSV envoyé sans débordement.");
}

void setup_wifi() {
  WiFi.softAP("IIS2DH-Logger", "12345678");
  Serial.print("📡 Connectez-vous à : "); Serial.println("IIS2DH-Logger");

  server.on("/", handleRoot);
  server.on("/start", handleStart);
  server.on("/download", handleDownload);
  server.begin();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.printf("✅ RAM libre : %d bytes\n", ESP.getFreeHeap());
  Serial.printf("✅ PSRAM libre : %d bytes\n", ESP.getFreePsram());

  current_config = {
    .mode = IIS2DH_MODE_HIGH_RES,
    .odr = IIS2DH_ODR_1344HZ,
    .scale = IIS2DH_SCALE_2G,
    .axes = IIS2DH_AXES_XYZ
  };

  iis2dh_init(current_config);
  // on n'active PAS d'interruption, on fait juste du polling

  setup_wifi();

  SPIFFS.begin(); // ou LittleFS.begin()
  server.serveStatic("sadal.png", SPIFFS, "/logo.png");

  Serial.println("✅ Système prêt");
}

void loop() {
  server.handleClient();

  if (acquiring) {
    // Check if new data is ready
    if (iis2dh_data_ready()) {
      uint32_t now = micros();

      if (sample_count < MAX_SAMPLES) {
        iis2dh_data_mg_t data;
        iis2dh_read_acceleration_mg(&data);

        samples[sample_count].timestamp_us = now - start_time;
        samples[sample_count].x = data.x;
        samples[sample_count].y = data.y;
        samples[sample_count].z = data.z;
        sample_count++;
      }

      if ((now - start_time) >= (acq_duration_ms * 1000UL)) {
        stop_acquisition();
      }
    }
  }
}


