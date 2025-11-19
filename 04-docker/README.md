# 04-docker: TP « Handle » Docker - 2 exercices - Durée approx. ~ 1h30min

Les réponses sont dans le fichier [SOLUTION.md](SOLUTION.md).

## I - Proposer un service

Le but de cet exercice est de mettre en œuvre un conteneur docker proposant un service Jenkins.

La démarche se déroule en quatre temps à savoir :

1.  La récupération de l'image jenkins depuis le hub docker.
2.  Le démarrage du conteneur proposant le service.
3.  La vérification de la disponibilité du service.
4.  L'arrêt du conteneur associé.

- Pour réaliser la récupération de l'image, téléchargez la dernière image docker **jenkins** depuis internet.
  - Vous pouvez alternativement charger l'image depuis un fichier avec la commande adéquate.
- Lors du démarrage du service **jenkins** veuillez penser à exposer le port associé au service afin de le rendre accessible depuis votre machine hôte.
- Une fois le conteneur démarré, vous devez être à même de pouvoir accéder au service depuis votre machine hôte grâce à une url de la syntaxe : `http://IP_MACHINE_HOTE:8080/`

## II - Service « from scratch »

Le but de cet exercice est de reconstruire un conteneur docker proposant un service Jenkins à partir d'une base Tomcat.

La démarche se déroule en quatre temps à savoir :

1.  La récupération des ressources.
2.  La rédaction d'un fichier Dockerfile.
3.  La construction de l'image.
4.  Le test du service.

- Pour réaliser la récupération de l'image, téléchargez les ressources nécessaires à la construction de l'image à savoir la dernière image docker **tomcat** ainsi que l'archive web (_war_) jenkins depuis internet.
  - Vous pouvez alternativement charger l'image depuis un fichier avec la commande adéquate.
  - Vous trouverez l'archive web de l'application jenkins sur le site <https://jenkins.io/>
- Afin de construire une nouvelle image docker appuyez-vous sur la rédaction d'un fichier Dockerfile en spécifiant :
  - Le fait que l'on se repose sur l'image **Tomcat** en tant que base de départ.
  - Le fait que l'archive **war** doit être disponible au sein de l'image générée.
  - Le fait que le port d'écoute **8080** réseau soit déclaré.
- Une fois le conteneur construit assurez-vous que celui-ci soit fonctionnel en démarrant ce dernier et vérifiant le fait de pouvoir accéder.
