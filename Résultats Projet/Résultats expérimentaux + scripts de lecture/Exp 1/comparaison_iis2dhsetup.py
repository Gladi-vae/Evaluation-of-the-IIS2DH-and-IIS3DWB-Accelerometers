import numpy as np
import matplotlib.pyplot as plt
from scipy.fft import fft, fftfreq
from scipy.signal import windows

# === CONFIGURATION ===
fs = 1330  # fréquence d'échantillonnage Hz
fichiers = {
    "Setup n°1": "4.1.1_iis2dh_setup1",
    "Setup n°2": "4.1.1_iis2dh_setup2"
}


def charger_donnees(fichier):
    with open(fichier, 'r') as f:
        lignes = f.readlines()[3:]  # saute les 3 premières lignes
    data = np.array([list(map(float, ligne.strip().split(','))) for ligne in lignes])

    # Extraction des colonnes
    temps = data[:, 0] / 1e6  # On suppose que c'est en microsecondes → conversion en secondes
    axes = {
        'X': data[:, 1],
        'Y': data[:, 2],
        'Z': data[:, 3]
    }
    duree = temps[-1] - temps[0]
    return temps, axes, duree


def analyser_fft(signaux, fs):
    resultat = {}
    for axe, signal in signaux.items():
        signal -= np.mean(signal)  # suppression composante continue
        signal *= windows.hann(len(signal))  # fenêtrage
        N = len(signal)
        fft_result = fft(signal)
        freqs = fftfreq(N, d=1 / fs)
        amplitude = np.abs(fft_result) / N
        mask = freqs > 0
        resultat[axe] = (freqs[mask], amplitude[mask])
    # choisir l'axe avec le pic max
    axe_dominant = max(resultat, key=lambda a: np.max(resultat[a][1]))
    return resultat[axe_dominant]


# === PLOT COMPARATIF ===
plt.figure(figsize=(6, 6))

for label, fichier in fichiers.items():
    temps, donnees, duree = charger_donnees(fichier)
    print(f"{label}: {len(temps)} échantillons — durée ≈ {duree:.3f} secondes")
    freqs, amp = analyser_fft(donnees, fs)
    plt.plot(freqs, amp, label=label)

plt.title("IIS2DH: Comparaison spectre FFT (axes dominants)")
plt.xlabel("Fréquence (Hz)")
plt.ylabel("Amplitude")
plt.grid(True)
#plt.xlim(0, fs / 2)
plt.xlim(400, 600)
plt.legend()
plt.tight_layout()
#plt.savefig("Résultats/iis2setup1_vs_setup2.png", dpi=300)
plt.show()
