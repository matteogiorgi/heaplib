#!/bin/sh

set -eu

usage()
{
    cat <<'EOF'
Usage: scripts/release.sh VERSION [--upload] [--repo OWNER/REPO]

Build, test, and package a pqlib release.

Arguments:
  VERSION            Release version, for example 0.1.0 or v0.1.0.

Options:
  --upload           Create or update the GitHub Release with dist/*.whl.
  --repo OWNER/REPO  GitHub repository passed to gh with -R.
  -h, --help         Show this help message.

Without --upload, this script only performs the local release checks and builds
the wheel under dist/.
EOF
}

VERSION=
UPLOAD=0
REPO=

while [ "$#" -gt 0 ]; do
    case "$1" in
        --upload)
            UPLOAD=1
            shift
            ;;
        --repo)
            if [ "$#" -lt 2 ]; then
                echo "error: --repo requires OWNER/REPO" >&2
                exit 2
            fi
            REPO=$2
            shift 2
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        -*)
            echo "error: unknown option: $1" >&2
            usage >&2
            exit 2
            ;;
        *)
            if [ -n "$VERSION" ]; then
                echo "error: version was provided more than once" >&2
                usage >&2
                exit 2
            fi
            VERSION=$1
            shift
            ;;
    esac
done

if [ -z "$VERSION" ]; then
    echo "error: missing VERSION" >&2
    usage >&2
    exit 2
fi

case "$VERSION" in
    v*) TAG=$VERSION ;;
    *) TAG=v$VERSION ;;
esac

GH_REPO_ARGS=
if [ -n "$REPO" ]; then
    GH_REPO_ARGS="-R $REPO"
fi

echo "==> Cleaning previous build artifacts"
make clean

echo "==> Running C tests"
make test

echo "==> Running Python tests"
make python-test

echo "==> Building wheel"
make wheel

echo "==> Built release artifacts"
ls -1 dist/*.whl

if [ "$UPLOAD" -eq 0 ]; then
    cat <<EOF

Local release build complete.

To upload these wheels manually:
  make release-upload VERSION=${TAG#v}

EOF
    exit 0
fi

if ! command -v gh >/dev/null 2>&1; then
    echo "error: --upload requires GitHub CLI (gh)" >&2
    exit 1
fi

echo "==> Ensuring git tag exists locally"
if ! git rev-parse "$TAG" >/dev/null 2>&1; then
    git tag "$TAG"
fi

echo "==> Pushing git tag"
# shellcheck disable=SC2086
git push origin "$TAG"

echo "==> Creating GitHub release or uploading to existing release"
# shellcheck disable=SC2086
if gh release view "$TAG" $GH_REPO_ARGS >/dev/null 2>&1; then
    # shellcheck disable=SC2086
    gh release upload "$TAG" dist/*.whl --clobber $GH_REPO_ARGS
else
    # shellcheck disable=SC2086
    gh release create "$TAG" dist/*.whl \
        --title "pqlib ${TAG#v}" \
        --notes "pqlib ${TAG#v} release." \
        $GH_REPO_ARGS
fi

echo "==> Release complete: $TAG"
