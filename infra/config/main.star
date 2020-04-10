#!/usr/bin/env lucicfg

luci.project(
    name = "swiftshader",
)

luci.cq_group(
    name = 'Main CQ',
    watch = cq.refset('https://swiftshader.googlesource.com/SwiftShader'),
    acls = [
        acl.entry(
            acl.CQ_COMMITTER,
            groups = 'Contributors',
        ),
        acl.entry(
            acl.CQ_DRY_RUNNER,
            groups = 'Contributors',
        ),
    ],
    verifiers = [
        luci.cq_tryjob_verifier(
            builder = 'chromium:try/linux-swangle-try-tot-swiftshader-x64',
        ),
        luci.cq_tryjob_verifier(
            builder = 'chromium:try/win-swangle-try-tot-swiftshader-x86',
        ),
    ],
)

