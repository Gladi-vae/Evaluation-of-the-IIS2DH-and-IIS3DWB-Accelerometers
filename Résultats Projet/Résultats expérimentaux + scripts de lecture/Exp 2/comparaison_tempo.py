import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import os


#IL SUFFIT DE CHANGER LES NOMS DES FICHIERS DANS CSV FILES POUR COMPARER LES 4 A 1m
# OU LES 4 A 30cm
# === Liste des fichiers CSV à comparer ===
csv_files = [
    "contreplaqué_30cm_planche.csv",
    "contreplaqué_30cm.csv",
    "bitume_30cm_planche.csv",
    "bitume_30cm.csv"
]

# === Légendes personnalisées pour chaque fichier (même ordre que csv_files) ===
custom_labels = [
    "planche+contreplaqué à 1m",
    "contreplaqué à 1m",
    "planche+bitume à 1m",
    "bitume à 1m"
]
# === Légendes personnalisées pour chaque fichier (même ordre que csv_files) ===
custom_labels = [
    "planche+contreplaqué à 30cm",
    "contreplaqué à 30cm",
    "planche+bitume à 30cm",
    "bitume à 30cm"
]


# === Couleurs pour les courbes ===
colors = ['red', 'green', 'blue', 'orange']

# === Tracé ===
plt.figure(figsize=(10, 10))

for i, path in enumerate(csv_files):
    # Lecture des données
    df = pd.read_csv(path, skiprows=8)
    t = df["t(us)"].values * 1e-6  # conversion µs → s
    x = df["X(mg)"].values
    y = df["Y(mg)"].values
    z = df["Z(mg)"].values
    norm = np.sqrt(x**2 + y**2 + z**2)

    # Utiliser la légende personnalisée
    label = custom_labels[i]
    plt.plot(t, norm, label=label, color=colors[i % len(colors)])

plt.xlabel("Temps (s)")
plt.ylabel("Vecteur accélération (mg)")
plt.title("Tests à 30cm: vecteurs accélération")
plt.xlim(2, 6)  # <=== Limite l’axe des x de 2 à 6 secondes
plt.legend()
plt.grid(True)
plt.tight_layout()
#plt.savefig("Résultats/comparaison_accel30cm.png", dpi=300)
plt.show()

