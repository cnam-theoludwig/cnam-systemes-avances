# Solution du TP 04-docker

## I - Proposer un service

### 1. La récupération de l'image

```bash
docker pull jenkins/jenkins:lts
```

### 2. Le démarrage du conteneur

- `-d` : lance le conteneur en arrière-plan (détaché), "daemon".
- `-p 8080:8080` : lie le port 8080 de la machine hôte au port 8080 du conteneur (port web de Jenkins).
- `-p 50000:50000` : lie le port 50000 de la machine hôte au port 50000 du conteneur (port pour les agents Jenkins).
- `--name jenkins-server` : donne un nom au conteneur pour le manipuler plus facilement plus tard.

```bash
docker run -d -p 8080:8080 -p 50000:50000 --name jenkins-server jenkins/jenkins:lts
```

### 3. La vérification de la disponibilité du service

```bash
docker ps
```

**Accès au service via le navigateur :** <http://localhost:8080/>

### 4. L'arrêt du conteneur associé

```bash
docker stop jenkins-server
```

Et pour supprimer le conteneur :

```bash
docker rm jenkins-server
```
