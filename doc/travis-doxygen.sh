#!/bin/bash
# Update Doxygen documentation after push to 'master'.
# Author: @pah, https://github.com/pah

set -e

DOXYGEN_VERSION=1.8.15
DOXYGEN_VER=doxygen-${DOXYGEN_VERSION}
DOXYGEN_TAR=${DOXYGEN_VER}.linux.bin.tar.gz
# ftp.stack.nl has been shutdown, so I changed to sourceforge.net.
# DOXYGEN_URL="http://ftp.stack.nl/pub/users/dimitri/${DOXYGEN_TAR}"
DOXYGEN_URL="https://sourceforge.net/projects/doxygen/files/rel-${DOXYGEN_VERSION}/${DOXYGEN_TAR}/download"

: ${GITHUB_REPO:="lreis2415/SEIMS"}
GITHUB_HOST="github.com"
GITHUB_CLONE="git://${GITHUB_HOST}/${GITHUB_REPO}"
GITHUB_URL="https://${GITHUB_HOST}/${GITHUB_PUSH-${GITHUB_REPO}}"

# if not set, ignore password
#GIT_ASKPASS="${TRAVIS_BUILD_DIR}/gh_ignore_askpass.sh"

skip() {
	echo "$@" 1>&2
	echo "Exiting..." 1>&2
	exit 0
}

abort() {
	echo "Error: $@" 1>&2
	echo "Exiting..." 1>&2
	exit 1
}

# TRAVIS_BUILD_DIR not set, exiting
[ -d "${TRAVIS_BUILD_DIR-/nonexistent}" ] || \
	abort '${TRAVIS_BUILD_DIR} not set or nonexistent.'

# check for pull-requests
[ "${TRAVIS_PULL_REQUEST}" = "false" ] || \
	skip "Not running Doxygen for pull-requests."

# check for branch name
#[ "${TRAVIS_BRANCH}" = "master" ] || \
#	skip "Running Doxygen only for updates on 'master' branch (current: ${TRAVIS_BRANCH})."

# check for job number
# [ "${TRAVIS_JOB_NUMBER}" = "${TRAVIS_BUILD_NUMBER}.1" ] || \
# 	skip "Running Doxygen only on first job of build ${TRAVIS_BUILD_NUMBER} (current: ${TRAVIS_JOB_NUMBER})."

# install doxygen binary distribution
doxygen_install()
{
	wget -O - "${DOXYGEN_URL}" | \
		tar xz -C ${TMPDIR-/tmp} ${DOXYGEN_VER}/bin/doxygen
    export PATH="${TMPDIR-/tmp}/${DOXYGEN_VER}/bin:$PATH"
}

doxygen_run()
{
	cd "${TRAVIS_BUILD_DIR}";
	doxygen ${TRAVIS_BUILD_DIR}/build/doc/Doxyfile;
	doxygen ${TRAVIS_BUILD_DIR}/build/doc/Doxyfile.zh-cn;
}

gh_pages_prepare()
{
	cd "${TRAVIS_BUILD_DIR}/build/doc";
	[ ! -d "html" ] || \
		abort "Doxygen target directory already exists."
	git --version
	git clone --single-branch -b gh-pages "${GITHUB_CLONE}" html
	cd html
	# setup git config (with defaults)
	git config user.name "${GIT_NAME-travis}"
	git config user.email "${GIT_EMAIL-"travis@localhost"}"
	# clean working dir
	rm -f .git/index
	git clean -df
}

gh_pages_commit() {
	cd "${TRAVIS_BUILD_DIR}/build/doc/html";
	# This line can be uncommented when we have a domain address.
	# echo "seims.lreis.ac.cn" > CNAME
	# In my case, I had folders whose names started with _ (like _css and _js),
	#   which GH Pages ignores as per Jekyll processing rules. If you don't use Jekyll,
	#   the workaround is to place a file named .nojekyll in the root directory.
	#   See https://stackoverflow.com/a/39691475
	touch .nojekyll
	git add --all;
	git diff-index --quiet HEAD || git commit -m "Automatic doxygen build";
}

gh_setup_askpass() {
	cat > ${GIT_ASKPASS} <<EOF
#!/bin/bash
echo
exit 0
EOF
	chmod a+x "$GIT_ASKPASS"
}

gh_pages_push() {
	# check for secure variables
	[ "${TRAVIS_SECURE_ENV_VARS}" = "true" ] || \
		skip "Secure variables not available, not updating GitHub pages."
	# check for GitHub access token
	[ "${GH_TOKEN+set}" = set ] || \
		skip "GitHub access token not available, not updating GitHub pages."
	[ "${#GH_TOKEN}" -eq 40 ] || \
		abort "GitHub token invalid: found ${#GH_TOKEN} characters, expected 40."

	cd "${TRAVIS_BUILD_DIR}/build/doc/html";
	# setup credentials (hide in "set -x" mode)
	git remote set-url --push origin "${GITHUB_URL}"
	git config credential.helper 'store'
	# ( set +x ; git config credential.username "${GH_TOKEN}" )
	( set +x ; [ -f ${HOME}/.git-credentials ] || \
			( echo "https://${GH_TOKEN}:@${GITHUB_HOST}" > ${HOME}/.git-credentials ; \
			 chmod go-rw ${HOME}/.git-credentials ) )
	# push to GitHub
	git push origin gh-pages
}

doxygen_install
gh_pages_prepare
doxygen_run
gh_pages_commit
gh_pages_push

