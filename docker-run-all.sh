#/bin/sh


docker build -t asayah/router -f Dockerfile-router .

docker run -d --rm --name backend-A -h backend-A --env TERM=xterm-color nginxdemos/hello:plain-text
docker run -d --rm --name backend-B -h backend-B --env TERM=xterm-color nginxdemos/hello:plain-text
docker run -d -t -h client --name client --env TERM=xterm-color ubuntu 
docker run -d --privileged -h router --name router --env TERM=xterm-color asayah/router