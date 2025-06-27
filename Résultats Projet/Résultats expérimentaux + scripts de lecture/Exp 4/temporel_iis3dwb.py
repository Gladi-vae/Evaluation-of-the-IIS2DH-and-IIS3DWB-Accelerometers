import pandas as pd
import matplotlib.pyplot as plt
import os

# === Paramètres ===
csv_path = "iis3dwb_coups_baton.csv"  # Remplace par le nom de ton fichier
nom_fichier = os.path.basename(csv_path)  # Extrait juste le nom (sans le chemin)
# === Lecture du fichier CSV ===
df = pd.read_csv(csv_path, skiprows=1)  # On saute la ligne de titre descriptive

# === Extraction et conversion des données ===
t = df["time[us]"].values * 1e-6  # Temps en secondes
x = df["acc_x[mg]"].values
y = df["acc_y[mg]"].values
z = df["acc_z[mg]"].values

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
#plt.savefig("../Résultats/passage2iis3tempo.png", dpi=300)
plt.show()