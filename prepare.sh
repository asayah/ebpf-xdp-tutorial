#/bin/sh
# docker asayah/xdp-tutorial-base -t base --no-cache . 

git submodule init
git submodule update

docker run -d --rm --name target-A -h target-A --env TERM=xterm-color nginxdemos/hello:plain-text
docker run -d --rm --name target-B -h target-B --env TERM=xterm-color nginxdemos/hello:plain-text
docker run -d -t -h client --name client --env TERM=xterm-color asayah/xdp-tutorial-base
docker run -d -t -h restricted-client --name restricted-client --env TERM=xterm-color asayah/xdp-tutorial-base
docker run -v $(pwd):/tutorial -d --privileged -h router --name router --env TERM=xterm-color asayah/xdp-tutorial-base