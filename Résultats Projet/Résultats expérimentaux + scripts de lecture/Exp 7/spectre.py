import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from scipy.interpolate import interp1d
from scipy.fft import fft, fftfreq

# === Paramètres ===
csv_path = "placecar12SP1m65k.csv"  # Remplace par ton chemin réel
duree_analyse = 4  # en secondes
fs_cible = 2000    # Fréquence cible d'interpolation (Hz)
titre_experience = "accélération à 65km/h"

# === Lecture du fichier et détection de l’entête ===
with open(csv_path, 'r') as f:
    lignes = f.readlines()

start_index = next(i for i, line in enumerate(lignes) if line.strip().startswith("t(us)"))
df = pd.read_csv(csv_path, skiprows=start_index)

# === Extraction du temps et signal Z ===
t_orig = df["t(us)"].values * 1e-6  # µs → s
z_orig = df["Z(mg)"].values

# === Interpolation sur un temps régulier ===
t_min = t_orig[0]
t_max = t_orig[-1]
#t_uniforme = np.arange(t_min, min(t_min+duree_analyse, t_max), 1 / fs_cible)
t_uniforme = np.arange(t_min, t_max, 1 / fs_cible)


interp_func = interp1d(t_orig, z_orig, kind='linear', bounds_error=False, fill_value="extrapolate")
z_interp = interp_func(t_uniforme)

# === Suppression de la composante continue et fenêtrage ===
z_centered = z_interp - np.mean(z_interp)
window = np.hanning(len(z_centered))
z_windowed = z_centered * window

# === FFT ===
N = len(z_windowed)
fft_result = fft(z_windowed)
freqs = fftfreq(N, d=1/fs_cible)
amplitudes = np.abs(fft_result) / N
positives = freqs > 0

# === Affichage ===
plt.figure(figsize=(10, 7))
plt.plot(freqs[positives], amplitudes[positives])
plt.title(f"FFT de Z - {titre_experience} (bande utile)")
plt.xlabel("Fréquence (Hz)")
plt.ylabel("Amplitude")
plt.grid(True)
plt.xlim(0, fs_cible / 40)
plt.tight_layout()
#plt.savefig("../Résultats/passageaccel65kspectre.png", dpi=300)
plt.show()
