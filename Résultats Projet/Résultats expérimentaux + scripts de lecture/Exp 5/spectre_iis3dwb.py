import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from scipy.interpolate import interp1d
from scipy.fft import fft, fftfreq

# === Paramètres ===
csv_path = "iis3dwb_coups_baton.csv"  # Remplace par le chemin réel
duree_analyse = 4  # Durée en secondes
fs_cible = 10000    # Fréquence d'échantillonnage cible (Hz)

# === Lecture du CSV en ignorant les lignes de commentaire ===
# On ignore la première ligne (# commentaire)
df = pd.read_csv(csv_path, skiprows=1)

# Nettoyage des noms de colonnes (en cas d'espaces ou de guillemets)
df.columns = df.columns.str.strip().str.replace('"', '').str.replace("'", "")

# === Vérification des colonnes ===
print("Colonnes disponibles :", df.columns.tolist())

# === Extraction du temps et du signal Z ===
t_orig = df["time[us]"].astype(float).values * 1e-6  # µs → secondes
z_orig = df["acc_z[mg]"].astype(float).values

# === Interpolation du signal à fréquence régulière ===
t_min = t_orig[0]
t_max = t_orig[-1]
t_uniforme = np.arange(t_min, min(t_min + duree_analyse, t_max), 1 / fs_cible)

interp_func = interp1d(t_orig, z_orig, kind='linear', bounds_error=False, fill_value="extrapolate")
z_interp = interp_func(t_uniforme)

# === Filtrage (centrage et fenêtrage) ===
z_centered = z_interp - np.mean(z_interp)
window = np.hanning(len(z_centered))
z_windowed = z_centered * window

# === FFT ===
N = len(z_windowed)
fft_result = fft(z_windowed)
freqs = fftfreq(N, d=1/fs_cible)
amplitudes = np.abs(fft_result) / N
positives = freqs > 0

# === Affichage du spectre ===
plt.figure(figsize=(10, 8))
plt.plot(freqs[positives], amplitudes[positives])
plt.title(f"IIS3DWB: FFT de Z")
plt.xlabel("Fréquence (Hz)")
plt.ylabel("Amplitude")
plt.grid(True)
plt.xlim(0, fs_cible / 2  )
plt.tight_layout()
#plt.savefig("../Résultats/passage2iis3spectre.png", dpi=300)
plt.show()
