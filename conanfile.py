from conan import ConanFile
from conan.tools.cmake import cmake_layout

class PulseVectorRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("sfml/3.0.2") # SFML 3.0.2
        self.requires("entt/3.14.0") # ECS
        self.requires("fmt/10.2.1") # Logging
        self.requires("nlohmann_json/3.11.3") # For potentially more complex metadata
        self.requires("gtest/1.14.0") # Testing framework

    def layout(self):
        cmake_layout(self)
