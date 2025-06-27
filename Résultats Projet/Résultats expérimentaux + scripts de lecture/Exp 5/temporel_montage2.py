import pandas as pd
import matplotlib.pyplot as plt
import os

# === Paramètre ===
csv_path = "montage2_bitume_voiture.csv"  # Remplace par le chemin de ton fichier CSV
nom_fichier = os.path.basename(csv_path)  # Extrait juste le nom (sans le chemin)
# === Lecture du fichier et détection de l’en-tête ===
with open(csv_path, 'r') as f:
    lignes = f.readlines()

# Cherche la ligne contenant "t(us)" pour déterminer où commencent les vraies données
start_index = next(i for i, line in enumerate(lignes) if line.strip().startswith("t(us)"))

# Lecture du DataFrame à partir de cette ligne
df = pd.read_csv(csv_path, skiprows=start_index)

# === Extraction des colonnes et conversion du temps ===
t = df["t(us)"].values * 1e-6  # µs → secondes
x = df["X(mg)"].values
y = df["Y(mg)"].values
z = df["Z(mg)"].values

# === Affichage sur 3 subplots ===
fig, axs = plt.subplots(3, 1, figsize=(10, 8), sharex=True)

axs[0].plot(t, x, color='red')
axs[0].set_ylabel("X (mg)")
#axs[0].set_title(f"Accélération X — {nom_fichier}")
axs[0].set_title(f"Accélération X - Passage voiture")

axs[1].plot(t, y, color='green')
axs[1].set_ylabel("Y (mg)")
#axs[1].set_title(f"Accélération Y — {nom_fichier}")
axs[1].set_title(f"Accélération Y - Passage voiture")

axs[2].plot(t, z, color='blue')
axs[2].set_ylabel("Z (mg)")
#axs[2].set_title(f"Accélération Z — {nom_fichier}")
axs[2].set_title(f"Accélération Z - Passage voiture")
axs[2].set_xlabel("Temps (s)")

plt.tight_layout()
#plt.savefig("../Résultats/passage2montage2tempo.png", dpi=300)
plt.show()