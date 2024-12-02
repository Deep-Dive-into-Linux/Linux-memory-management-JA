sudo dnf groupinstall -y "Development Tools"

# https://www.redhat.com/en/blog/install-epel-linux
sudo subscription-manager repos --enable codeready-builder-for-rhel-9-$(arch)-rpms
sudo dnf install \
    https://dl.fedoraproject.org/pub/epel/epel-release-latest-9.noarch.rpm

sudo dnf install -y bc dwarves elfutils-libelf-devel openssl-devel rsync ncurses-devel
