import pandas as pd
import matplotlib.pyplot as plt

# === Liste des fichiers CSV à comparer ===
files = [
    "contreplaqué_30cm.csv",
    "contreplaqué_1m.csv",
    "bitume_30cm.csv",
    "bitume_1m.csv"
]

labels = [
    "Contreplaqué 30cm",
    "Contreplaqué 1m",
    "Bitume 30cm",
    "Bitume 1m"
]

colors = ["blue", "green", "orange", "red"]

plt.figure(figsize=(12, 6))

for file, label, color in zip(files, labels, colors):
    # Lecture du fichier avec saut de lignes d'en-tête
    df = pd.read_csv(file, skiprows=8)

    # Extraction des données
    t_us = df["t(us)"].values * 1e-6  # conversion µs → s
    z = df["Z(mg)"].values

    # Tracé
    plt.plot(t_us, z, label=label, color=color, linewidth=1)

# Mise en forme du graphique
plt.xlabel("Temps (s)")
plt.ylabel("Accélération Z (mg)")
plt.title("Comparaison des accélérations en Z")
plt.legend()
plt.grid(True)
plt.xlim(2, 7)
plt.tight_layout()
plt.savefig("Résultats/comparaison3pattes.png", dpi=300)
plt.show()
