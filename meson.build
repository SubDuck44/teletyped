project(
    'teletyped',
    'c',
    default_options: {
        'c_std': 'c23',
        'buildtype': 'debugoptimized',
        'warning_level': '3',
    },
)

executable(
    'teletyped',
    sources: [
        run_command('find', 'src', '-name', '*.c', check: true).stdout().split(),
    ],
    c_args: [
        '-DVERSION="@0@"'.format(meson.project_version()),
        '-D_POSIX_C_SOURCE=200809L',
        '-Wno-empty-translation-unit',
        '-Wno-pragma-once-outside-header',
    ],
    link_args: [],
    dependencies: [
        dependency('libev'),
        dependency('sdl3'),
    ],
    install: true,
)
