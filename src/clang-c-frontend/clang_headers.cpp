#include <clang-c-frontend/clang_c_language.h>
#include <fstream>
#include <ac_config.h>

#ifdef ESBMC_CLANG_HEADERS_BUNDLED
extern "C"
{
#include <headers/cheaders.h> /* generated by build system */
#undef ESBMC_FLAIL
}
#endif

const std::string &clang_c_languaget::clang_headers_path()
{
#ifdef ESBMC_CLANG_HEADERS_BUNDLED
  // Dump clang headers into a temporary directory
  static bool dumped = false;
  /* About the path being static:
   * the function dump_clang_headers has a static member checking if it was
   * ever extracted before. This will guarantee that the same path will be used
   * during a run. And no more than one is required anyway */
  static auto p =
    file_operations::create_tmp_dir("esbmc-headers-%%%%-%%%%-%%%%");
  if(!dumped)
  {
    dumped = true;
#define ESBMC_FLAIL(body, size, ...)                                           \
  std::ofstream(p.path() + "/" + #__VA_ARGS__).write(body, size);
#include <headers/cheaders.h>
#undef ESBMC_FLAIL
  }
  return p.path();
#else
  // clang headers not bundled, return the path set at compile time
  static const std::string path = ESBMC_CLANG_HEADER_DIR;
  return path;
#endif
}
