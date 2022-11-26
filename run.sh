#/bin/sh

docker build -t base .

docker run -d --rm --name target-A -h target-A --env TERM=xterm-color nginxdemos/hello:plain-text
docker run -d --rm --name target-B -h target-B --env TERM=xterm-color nginxdemos/hello:plain-text
docker run -d -t -h client --name client --env TERM=xterm-color base
docker run -d -t -h restricted-client --name restricted-client --env TERM=xterm-color base
docker run -d --privileged -h router --name router --env TERM=xterm-color base