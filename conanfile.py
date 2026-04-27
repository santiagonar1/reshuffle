from conan import ConanFile


class MyProjectConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"

    def requirements(self):
        self.requires("gtest/1.17.0")
        self.requires("abseil/20260107.1")
        self.requires("benchmark/1.9.5")
        self.requires("tracy/0.13.1")
        self.requires("zpp_bits/4.4.25")

    def configure(self):
        self.options["gtest/*"].build_gmock = True
