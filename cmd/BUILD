load("@rules_pkg//pkg:tar.bzl", "pkg_tar")

pkg_tar(
    name = "go_tools",
    srcs = [
        "//cmd/{}:{}".format(cmd, cmd)
        for cmd in [
            "lg",
            "livegrep",
            "livegrep-fetch-reindex",
            "livegrep-github-reindex",
            "livegrep-reload",
        ]
    ],
    package_dir = "/bin",
    visibility = ["//visibility:public"],
)
