add_rules("mode.debug", "mode.release")

set_languages("c++20")
set_warnings("allextra")
set_toolchains("clang")
add_includedirs("source")

if is_mode("debug") then
    set_symbols("debug")
    set_optimize("none")
end

target("u", {
    kind = "static",
    files = "source/u/**.cpp",
    pcxxheader = "source/u/precompile.h",
    optimize = "faster",
})

target("tests", {
    kind = "binary",
    deps = "u",
    files = "tests/*.cpp",
    optimize = "faster",
})
