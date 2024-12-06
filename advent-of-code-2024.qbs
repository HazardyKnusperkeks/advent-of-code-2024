import qbs

Project {
    name: "Advent of Code 2024"

    references: ["allWarnings.qbs"]

    CppApplication {
        consoleApplication: true
        files: [
            "3rdParty/ctre/include/**/*.hpp",
            "challenge1.cpp",
            "challenge1.hpp",
            "challenge2.cpp",
            "challenge2.hpp",
            "challenge3.cpp",
            "challenge3.hpp",
            "challenge4.cpp",
            "challenge4.hpp",
            "challenge5.cpp",
            "challenge5.hpp",
            "challenge6.cpp",
            "challenge6.hpp",
            "coordinate3d.hpp",
            "helper.cpp",
            "helper.hpp",
            "main.cpp",
            "print.cpp",
            "print.hpp",
        ]

        Depends { name: "AllWarnings" }
        Depends { name: "cpp" }

        cpp.cxxLanguageVersion: "c++26"
        cpp.cxxFlags: ["-fconcepts-diagnostics-depth=10"]
    }

    Product {
        files: ["data/*.txt"]
        name: "Data"
    }
}
