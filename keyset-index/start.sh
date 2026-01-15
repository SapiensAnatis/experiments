podman kube play ./postgres.yaml --build && podman logs -f keyset-index-postgres

function cleanup {
  echo "Stopping container"
  podman kube down ./postgres.yaml
}

trap cleanup EXIT