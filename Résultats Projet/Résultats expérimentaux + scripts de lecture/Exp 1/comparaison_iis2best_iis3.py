import numpy as np
import matplotlib.pyplot as plt
from scipy.fft import fft, fftfreq
from scipy.signal import windows

# === CONFIGURATION ===
fs_iis2dh = 1330   # Fréquence d'échantillonnage pour IIS2DH
fs_iis3dwb = 26900 # Fréquence d'échantillonnage pour IIS3DWB
duree_max = 0.950  # Durée max à analyser
fichier_iis2dh = "4.1.1_iis2dh_setup1.csv"
fichier_iis3dwb = "4.1.1_iis3dwb.csv"

def charger_et_analyser(fichier, fs, label):
    with open(fichier, 'r') as f:
        lignes = f.readlines()[3:]  # saute les 3 premières lignes

    data = np.array([list(map(float, ligne.strip().split(','))) for ligne in lignes])
    temps = data[:, 0] / 1e6  # µs → s
    t0 = temps[0]
    masque_duree = (temps - t0) <= duree_max
    temps = temps[masque_duree]
    data = data[masque_duree]

    if len(temps) == 0:
        raise ValueError(f"Aucun échantillon retenu dans {label} sur {duree_max:.3f} s")

    fs_moy = 1 / np.mean(np.diff(temps))
    print(f"{label}: {len(temps)} échantillons — durée = {temps[-1] - temps[0]:.3f} s — fs ≈ {fs_moy:.1f} Hz")

    axes = {
        'X': data[:, 1],
        'Y': data[:, 2],
        'Z': data[:, 3]
    }

    fft_data = {}
    for nom, signal in axes.items():
        signal_centered = signal - np.mean(signal)
        window = windows.hann(len(signal))
        signal_windowed = signal_centered * window
        N = len(signal_windowed)
        fft_result = fft(signal_windowed)
        freqs = fftfreq(N, d=1/fs)
        amplitudes = np.abs(fft_result) / N
        mask = freqs > 0
        fft_data[nom] = (freqs[mask], amplitudes[mask])

    # axe avec le plus fort pic
    axe_dom = max(fft_data, key=lambda k: np.max(fft_data[k][1]))
    freqs_dom, amp_dom = fft_data[axe_dom]
    print(f"  Axe dominant {label}: {axe_dom} — Pic à {freqs_dom[np.argmax(amp_dom)]:.2f} Hz")
    return freqs_dom, amp_dom, label

# === ANALYSE DES DEUX CAPTEURS ===
f_iis2, a_iis2, label2 = charger_et_analyser(fichier_iis2dh, fs_iis2dh, "IIS2DH (Setup 1)")
f_iis3, a_iis3, label3 = charger_et_analyser(fichier_iis3dwb, fs_iis3dwb, "IIS3DWB")

# === AFFICHAGE COMPARATIF ===
plt.figure(figsize=(6, 6))
plt.plot(f_iis2, a_iis2, label=label2)
plt.plot(f_iis3, a_iis3, label=label3)
plt.title("Comparaison FFT (axes dominants, durée = 0.950 s)")
plt.xlabel("Fréquence (Hz)")
plt.ylabel("Amplitude normalisée")
plt.grid(True)
plt.xlim(400, 600)
plt.legend()
plt.tight_layout()
#plt.savefig("Résultats/iis2setup1_vs_iis3.png", dpi=300)
plt.show()
