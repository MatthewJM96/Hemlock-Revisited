from conan import ConanFile


class HemlockRevisitedRecipe(ConanFile):
    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    options = {
        "with_opengl": [True, False],
        "with_lua": [True, False]
    }
    default_options = {
        "with_opengl": False,
        "with_lua": False
    }

    name = "hemlock"
    version = "0.0.1"

    def requirements(self):
        self.requires("glm/0.9.9.8")
        self.requires("sdl/2.28.3")
        self.requires("sdl_ttf/2.24.0")
        self.requires("libpng/1.6.50")
        self.requires("concurrentqueue/1.0.4")
        self.requires("boost/1.89.0")
        self.requires("fastnoise2/0.10.0-alpha")
        self.requires("bullet3/3.24")
        self.requires("entt/3.15.0")
        self.requires("yaml-cpp/0.8.0")
        self.requires("zlib/1.3.1")

        if self.options.with_opengl and self.settings.os != "Macos":
            self.requires("glew/2.2.0")

        if self.options.with_lua:
            self.requires("luajit/2.1.0-beta3")

    def configure(self):
        # Disable PulseAudio when building SDL, we really don't need this and it is a pain in the ass both for time of
        # fresh compiles and for generating build errors...
        self.options["sdl"].pulse = False
        # Wayland is also causing a build error right now...
        self.options["sdl"].wayland = False
