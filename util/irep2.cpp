#include <stdarg.h>
#include <string.h>

#include "irep2.h"
#include "std_types.h"
#include "migrate.h"

#include <solvers/prop/prop_conv.h>

#include <boost/algorithm/string.hpp>
#include <boost/static_assert.hpp>

template <class T>
list_of_memberst
tostring_func(unsigned int indent, const char *name, const T *val, ...)
{
  va_list list;

  list_of_memberst thevector;

  std::string stringval = (*val)->pretty(indent + 2);
  thevector.push_back(std::pair<std::string,std::string>
                               (std::string(name), stringval));

  va_start(list, val);
  do {
    const char *listname = va_arg(list, const char *);
    if (strlen(listname) == 0)
      return thevector;

    const T *v2 = va_arg(list, const T *);
    stringval = (*v2)->pretty(indent + 2);
    thevector.push_back(std::pair<std::string,std::string>
                                 (std::string(name), stringval));
  } while (1);
}

std::string
indent_str(unsigned int indent)
{
  return std::string(indent, ' ');
}

template <class T>
std::string
pretty_print_func(unsigned int indent, std::string ident, T obj)
{
  list_of_memberst memb = obj.tostring(indent);

  std::string indentstr = indent_str(indent);
  std::string exprstr = ident;

  for (list_of_memberst::const_iterator it = memb.begin(); it != memb.end();
       it++) {
    exprstr += "\n" + indentstr + it->first + " : " + it->second;
  }

  return exprstr;
}

/*************************** Base type2t definitions **************************/

type2t::type2t(type_ids id)
  : type_id(id)
{
}

bool
type2t::operator==(const type2t &ref) const
{

  return cmp(ref);
}

bool
type2t::operator!=(const type2t &ref) const
{

  return !(*this == ref);
}

bool
type2t::operator<(const type2t &ref) const
{
  int tmp = type2t::lt(ref);
  if (tmp < 0)
    return true;
  else if (tmp > 0)
    return false;
  else
    return (lt(ref) < 0);
}

int
type2t::ltchecked(const type2t &ref) const
{
  int tmp = type2t::lt(ref);
  if (tmp != 0)
    return tmp;

  return lt(ref);
}

bool
type2t::cmp(const type2t &ref) const
{

  if (type_id == ref.type_id)
    return true;
  return false;
}

int
type2t::lt(const type2t &ref) const
{

  if (type_id < ref.type_id)
    return -1;
  if (type_id > ref.type_id)
    return 1;
  return 0;
}

std::string
type2t::pretty(unsigned int indent) const
{

  return pretty_print_func<const type2t&>(indent, type_names[type_id], *this);
}

void
type2t::dump(void) const
{
  std::cout << pretty(0) << std::endl;
  return;
}

uint32_t
type2t::crc(void) const
{
  boost::crc_32_type crc;
  do_crc(crc);
  return crc.checksum();
}

void
type2t::do_crc(boost::crc_32_type &crc) const
{
  crc.process_byte(type_id);
  return;
}

const char *type2t::type_names[] = {
  "bool",
  "empty",
  "symbol",
  "struct",
  "union",
  "code",
  "array",
  "pointer",
  "unsignedbv",
  "signedbv",
  "fixedbv",
  "string"
};

template<class derived>
void
type_body<derived>::convert_smt_type(const prop_convt &obj, void *&arg) const
{
  const derived *derived_this = static_cast<const derived *>(this);
  obj.convert_smt_type(*derived_this, arg);
}

template<class derived>
void
bv_type_body<derived>::convert_smt_type(const prop_convt &obj, void *&arg) const
{
  const derived *derived_this = static_cast<const derived *>(this);
  obj.convert_smt_type(*derived_this, arg);
}

template<class derived>
void
struct_union_type_body2t<derived>::convert_smt_type(const prop_convt &obj, void *&arg) const
{
  const derived *derived_this = static_cast<const derived *>(this);
  obj.convert_smt_type(*derived_this, arg);
}

bv_type2t::bv_type2t(type2t::type_ids id, unsigned int _width)
  : type_body<bv_type2t>(id),
    width(_width)
{
}

unsigned int
bv_type2t::get_width(void) const
{
  return width;
}

bool
bv_type2t::cmp(const bv_type2t &ref) const
{
  if (width == ref.width)
    return true;
  return false;
}

int
bv_type2t::lt(const type2t &ref) const
{
  const bv_type2t &ref2 = static_cast<const bv_type2t &>(ref);

  if (width < ref2.width)
    return -1;
  if (width > ref2.width)
    return 1;

  return 0;
}

list_of_memberst
bv_type2t::tostring(unsigned int indent) const
{
  char bees[256];
  list_of_memberst membs;

  snprintf(bees, 255, "%d", width);
  bees[255] = '\0';
  membs.push_back(member_entryt("width", bees));
  return membs;
}

void
bv_type2t::do_crc(boost::crc_32_type &crc) const
{
  type2t::do_crc(crc);
  crc.process_bytes(&width, sizeof(width));
  return;
}

struct_union_type2t::struct_union_type2t(type_ids id,
                                         const std::vector<type2tc> &_members,
                                         std::vector<std::string> memb_names,
                                         std::string _name)
  : type_body<struct_union_type2t>(id), members(_members),
                                        member_names(memb_names),
                                        name(_name)
{
}

bool
struct_union_type2t::cmp(const struct_union_type2t &ref) const
{

  if (name != ref.name)
    return false;

  if (members.size() != ref.members.size())
    return false;

  if (members != ref.members)
    return false;

  if (member_names != ref.member_names)
    return false;

  return true;
}

int
struct_union_type2t::lt(const type2t &ref) const
{
  const struct_union_type2t &ref2 =
                            static_cast<const struct_union_type2t &>(ref);

  if (name < ref2.name)
    return -1;
  if (name > ref2.name)
    return 1;

  if (members.size() < ref2.members.size())
    return -1;
  if (members.size() > ref2.members.size())
    return 1;

  if (members < ref2.members)
    return -1;
  if (members > ref2.members)
    return 1;

  if (member_names < ref2.member_names)
    return -1;
  if (member_names > ref2.member_names)
    return 1;

  return 0;
}

list_of_memberst
struct_union_type2t::tostring(unsigned int indent) const
{
  char bees[256];
  list_of_memberst membs;

  membs.push_back(member_entryt("struct name", name));

  unsigned int i = 0;
  forall_types(it, members) {
    snprintf(bees, 255, "member \"%s\" (%d)", member_names[i].c_str(), i);
    bees[255] = '\0';
    membs.push_back(member_entryt(std::string(bees), (*it)->pretty(indent +2)));
  }

  return membs;
}

void
struct_union_type2t::do_crc(boost::crc_32_type &crc) const
{
  type2t::do_crc(crc);
  crc.process_bytes(name.c_str(), name.size());

  forall_types(it, members)
    (*it)->do_crc(crc);

  for (std::vector<std::string>::const_iterator it = member_names.begin();
       it != member_names.end(); it++)
    crc.process_bytes(it->c_str(), it->size());

  return;
}

bool_type2t::bool_type2t(void)
  : type_body<bool_type2t>(bool_id)
{
}

unsigned int
bool_type2t::get_width(void) const
{
  return 1;
}

bool
bool_type2t::cmp(const bool_type2t &ref __attribute__((unused))) const
{
  return true; // No data stored in bool type
}

int
bool_type2t::lt(const type2t &ref __attribute__((unused))) const
{
  return 0; // No data stored in bool type
}

list_of_memberst
bool_type2t::tostring(unsigned int indent) const
{
  return list_of_memberst(); // No data here
}

signedbv_type2t::signedbv_type2t(unsigned int width)
  : bv_type_body<signedbv_type2t>(signedbv_id, width)
{
}

bool
signedbv_type2t::cmp(const signedbv_type2t &ref) const
{
  return bv_type2t::cmp(ref);
}

int
signedbv_type2t::lt(const type2t &ref) const
{
  return bv_type2t::lt(ref);
}

list_of_memberst
signedbv_type2t::tostring(unsigned int indent) const
{
  return bv_type2t::tostring(indent);
}

unsignedbv_type2t::unsignedbv_type2t(unsigned int width)
  : bv_type_body<unsignedbv_type2t>(unsignedbv_id, width)
{
}

bool
unsignedbv_type2t::cmp(const unsignedbv_type2t &ref) const
{
  return bv_type2t::cmp(ref);
}

int
unsignedbv_type2t::lt(const type2t &ref) const
{
  return bv_type2t::lt(ref);
}

list_of_memberst
unsignedbv_type2t::tostring(unsigned int indent) const
{
  return bv_type2t::tostring(indent);
}

array_type2t::array_type2t(const type2tc t, const expr2tc s, bool inf)
  : type_body<array_type2t>(array_id), subtype(t), array_size(s),
    size_is_infinite(inf)
{
}

unsigned int
array_type2t::get_width(void) const
{
  // Two edge cases: the array can have infinite size, or it can have a dynamic
  // size that's determined by the solver.
  if (size_is_infinite)
    throw new inf_sized_array_excp();

  if (array_size->expr_id != expr2t::constant_int_id)
    throw new dyn_sized_array_excp(array_size);

  // Otherwise, we can multiply the size of the subtype by the number of elements.
  unsigned int sub_width = subtype->get_width();

  expr2t *elem_size = array_size.get();
  constant_int2t *const_elem_size = dynamic_cast<constant_int2t*>(elem_size);
  assert(const_elem_size != NULL);
  unsigned long num_elems = const_elem_size->as_ulong();

  return num_elems * sub_width;
}

bool
array_type2t::cmp(const array_type2t &ref) const
{

  // Check subtype type matches
  if (subtype != ref.subtype)
    return false;

  // If both sizes are infinite, we're the same type
  if (size_is_infinite && ref.size_is_infinite)
    return true;

  // If only one size is infinite, then we're not the same, and liable to step
  // on a null pointer if we access array_size.
  if (size_is_infinite || ref.size_is_infinite)
    return false;

  // Otherwise,
  return (array_size == ref.array_size);
}

int
array_type2t::lt(const type2t &ref) const
{
  const array_type2t &ref2 = static_cast<const array_type2t&>(ref);

  int tmp = subtype->ltchecked(*ref2.subtype.get());
  if (tmp != 0)
    return tmp;

  if (size_is_infinite < ref2.size_is_infinite)
    return -1;
  if (size_is_infinite > ref2.size_is_infinite)
    return 1;

  if (size_is_infinite)
    return 0; // Both are infinite; and array_size is null.

  return array_size->ltchecked(*ref2.array_size.get());
}

list_of_memberst
array_type2t::tostring(unsigned int indent) const
{
  list_of_memberst membs = tostring_func<type2tc> (indent,
                                                  (const char *)"subtype",
                                                  &subtype, (const char *)"");

  if (size_is_infinite) {
    membs.push_back(member_entryt("size", "inifinite"));
  } else {
    list_of_memberst membs2 = tostring_func<expr2tc> (indent,
                                                     (const char *)"size",
                                                     &array_size,
                                                     (const char *)"");
    membs.push_back(membs2[0]);
  }

  return membs;
}

void
array_type2t::do_crc(boost::crc_32_type &crc) const
{
  type2t::do_crc(crc);
  subtype->do_crc(crc);
  if (size_is_infinite) {
    crc.process_byte(1);
  } else {
    crc.process_byte(0);
    array_size->do_crc(crc);
  }
}

pointer_type2t::pointer_type2t(type2tc _sub)
  : type_body<pointer_type2t>(pointer_id), subtype(_sub)
{
}

unsigned int
pointer_type2t::get_width(void) const
{
  return config.ansi_c.pointer_width;
}

bool
pointer_type2t::cmp(const pointer_type2t &ref) const
{

  return subtype == ref.subtype;
}

int
pointer_type2t::lt(const type2t &ref) const
{
  const pointer_type2t &ref2 = static_cast<const pointer_type2t&>(ref);

  return subtype->ltchecked(*ref2.subtype.get());
}

list_of_memberst
pointer_type2t::tostring(unsigned int indent) const
{
  list_of_memberst membs = tostring_func<type2tc> (indent,
                                                  (const char *)"subtype",
                                                  &subtype, (const char *)"");
  return membs;
}

void
pointer_type2t::do_crc(boost::crc_32_type &crc) const
{
  type2t::do_crc(crc);
  subtype->do_crc(crc);
  return;
}

empty_type2t::empty_type2t(void)
  : type_body<empty_type2t>(empty_id)
{
}

unsigned int
empty_type2t::get_width(void) const
{
  throw new symbolic_type_excp();
}

bool
empty_type2t::cmp(const empty_type2t &ref __attribute__((unused))) const
{
  return true; // Two empty types always compare true.
}

int
empty_type2t::lt(const type2t &ref __attribute__((unused))) const
{
  return 0; // Two empty types always same.
}

list_of_memberst
empty_type2t::tostring(unsigned int indent) const
{
  return list_of_memberst(); // No data here
}

symbol_type2t::symbol_type2t(const dstring sym_name)
  : type_body<symbol_type2t>(symbol_id), symbol_name(sym_name)
{
}

unsigned int
symbol_type2t::get_width(void) const
{
  assert(0 && "Fetching width of symbol type - invalid operation");
}

bool
symbol_type2t::cmp(const symbol_type2t &ref) const
{
  return symbol_name == ref.symbol_name;
}

int
symbol_type2t::lt(const type2t &ref) const
{
  const symbol_type2t &ref2 = static_cast<const symbol_type2t &>(ref);

  if (symbol_name < ref2.symbol_name)
    return -1;
  if (ref2.symbol_name < symbol_name)
    return 1;
  return 0;
}

list_of_memberst
symbol_type2t::tostring(unsigned int indent) const
{
  list_of_memberst membs;
  membs.push_back(member_entryt("symbol", symbol_name.as_string()));
  return membs;
}

void
symbol_type2t::do_crc(boost::crc_32_type &crc) const
{
  type2t::do_crc(crc);
  crc.process_bytes(symbol_name.c_str(), symbol_name.size());
  return;
}

struct_type2t::struct_type2t(std::vector<type2tc> &members,
                             std::vector<std::string> memb_names,
                             std::string name)
  : struct_union_type_body2t<struct_type2t>(struct_id, members, memb_names, name)
{
}

unsigned int
struct_type2t::get_width(void) const
{
  // Iterate over members accumulating width.
  std::vector<type2tc>::const_iterator it;
  unsigned int width = 0;
  for (it = members.begin(); it != members.end(); it++)
    width += (*it)->get_width();

  return width;
}

bool
struct_type2t::cmp(const struct_type2t &ref) const
{

  return struct_union_type2t::cmp(ref);
}

int
struct_type2t::lt(const type2t &ref) const
{

  return struct_union_type2t::lt(ref);
}

list_of_memberst
struct_type2t::tostring(unsigned int indent) const
{
  return struct_union_type2t::tostring(indent);
}

union_type2t::union_type2t(std::vector<type2tc> &members,
                           std::vector<std::string> memb_names,
                           std::string name)
  : struct_union_type_body2t<union_type2t>(union_id, members, memb_names, name)
{
}

unsigned int
union_type2t::get_width(void) const
{
  // Iterate over members accumulating width.
  std::vector<type2tc>::const_iterator it;
  unsigned int width = 0;
  for (it = members.begin(); it != members.end(); it++)
    width = std::max(width, (*it)->get_width());

  return width;
}

bool
union_type2t::cmp(const union_type2t &ref) const
{

  return struct_union_type2t::cmp(ref);
}

int
union_type2t::lt(const type2t &ref) const
{

  return struct_union_type2t::lt(ref);
}

list_of_memberst
union_type2t::tostring(unsigned int indent) const
{
  return struct_union_type2t::tostring(indent);
}

fixedbv_type2t::fixedbv_type2t(unsigned int _width, unsigned int integer)
  : type_body<fixedbv_type2t>(fixedbv_id), width(_width),
                                           integer_bits(integer)
{
}

unsigned int
fixedbv_type2t::get_width(void) const
{
  return width;
}

bool
fixedbv_type2t::cmp(const fixedbv_type2t &ref) const
{

  if (width != ref.width)
    return false;

  if (integer_bits != ref.integer_bits)
    return false;

  return true;
}

int
fixedbv_type2t::lt(const type2t &ref) const
{
  const fixedbv_type2t &ref2 = static_cast<const fixedbv_type2t &>(ref);

  if (width < ref2.width)
    return -1;
  if (width > ref2.width)
    return 1;

  if (integer_bits < ref2.integer_bits)
    return -1;
  if (integer_bits > ref2.integer_bits)
    return 1;

  return 0;
}

list_of_memberst
fixedbv_type2t::tostring(unsigned int indent) const
{
  char buffer[256];
  list_of_memberst membs;

  snprintf(buffer, 255, "%d", width);
  buffer[255] = '\0';
  membs.push_back(member_entryt("width", buffer));

  snprintf(buffer, 255, "%d", integer_bits);
  buffer[255] = '\0';
  membs.push_back(member_entryt("integer_bits", buffer));

  return membs;
}

void
fixedbv_type2t::do_crc(boost::crc_32_type &crc) const
{
  type2t::do_crc(crc);
  crc.process_bytes(&width, sizeof(width));
  crc.process_bytes(&integer_bits, sizeof(integer_bits));
  return;
}

code_type2t::code_type2t(void)
  : type_body<code_type2t>(code_id)
{
}

unsigned int
code_type2t::get_width(void) const
{
  throw new symbolic_type_excp();
}

bool
code_type2t::cmp(const code_type2t &ref __attribute__((unused))) const
{
  return true; // All code is the same. Ish.
}

int
code_type2t::lt(const type2t &ref __attribute__((unused))) const
{
  return 0; // All code is the same. Ish.
}

list_of_memberst
code_type2t::tostring(unsigned int indent) const
{
  return list_of_memberst();
}

string_type2t::string_type2t(unsigned int _elements)
  : type_body<string_type2t>(string_id), elements(_elements)
{
}

unsigned int
string_type2t::get_width(void) const
{
  return elements * 8;
}

bool
string_type2t::cmp(const string_type2t &ref) const
{
  return (elements == ref.elements);
}

int
string_type2t::lt(const type2t &ref) const
{
  const string_type2t &ref2 = static_cast<const string_type2t &>(ref);
  if (elements < ref2.elements)
    return -1;
  else if (elements > ref2.elements)
    return 1;
  return 0;
}

list_of_memberst
string_type2t::tostring(unsigned int indent) const
{
  return list_of_memberst();
}

void
string_type2t::do_crc(boost::crc_32_type &crc) const
{
  type2t::do_crc(crc);
  crc.process_bytes(&elements, sizeof(elements));
  return;
}

/*************************** Base expr2t definitions **************************/

expr2t::expr2t(const type2tc _type, expr_ids id)
  : expr_id(id), type(_type)
{
}

expr2t::expr2t(const expr2t &ref)
  : expr_id(ref.expr_id),
    type(ref.type)
{
}

void expr2t::convert_smt(prop_convt &obj, void *&arg) const
{ obj.convert_smt_expr(*this, arg); }


bool
expr2t::operator==(const expr2t &ref) const
{
  if (!expr2t::cmp(ref))
    return false;

  return cmp(ref);
}

bool
expr2t::operator!=(const expr2t &ref) const
{
  return !(*this == ref);
}

bool
expr2t::operator<(const expr2t &ref) const
{
  int tmp = expr2t::lt(ref);
  if (tmp < 0)
    return true;
  else if (tmp > 0)
    return false;
  else
    return (lt(ref) < 0);
}

int
expr2t::ltchecked(const expr2t &ref) const
{
  int tmp = expr2t::lt(ref);
  if (tmp != 0)
    return tmp;

  return lt(ref);
}

bool
expr2t::cmp(const expr2t &ref) const
{
  if (expr_id != ref.expr_id)
    return false;

  if (type != ref.type)
    return false;

  return true;
}

int
expr2t::lt(const expr2t &ref) const
{
  if (expr_id < ref.expr_id)
    return -1;
  if (expr_id > ref.expr_id)
    return 1;

  return type->ltchecked(*ref.type.get());
}

uint32_t
expr2t::crc(void) const
{
  boost::crc_32_type crc;
  do_crc(crc);
  return crc.checksum();
}

void
expr2t::do_crc(boost::crc_32_type &crc) const
{
  crc.process_byte(expr_id);
  type->do_crc(crc);
  return;
}

const char *expr2t::expr_names[] = {
  "constant_int",
  "constant_fixedbv",
  "constant_bool",
  "constant_string",
  "constant_struct",
  "constant_union",
  "constant_array",
  "constant_array_of",
  "symbol",
  "typecast",
  "if",
  "equality",
  "notequal",
  "lessthan",
  "greaterthan",
  "lessthanequal",
  "greaterthanequal",
  "not",
  "and",
  "or",
  "xor",
  "bitand",
  "bitor",
  "bitxor",
  "bitnand",
  "bitnor",
  "bitnxor",
  "lshr",
  "neg",
  "abs",
  "add",
  "sub",
  "mul",
  "div",
  "modulus",
  "shl",
  "ashr",
  "dynamic_object",
  "same_object",
  "pointer_offset",
  "pointer_object",
  "address_of",
  "byte_extract",
  "byte_update",
  "with",
  "member",
  "index",
  "zero_string",
  "zero_length_string",
  "isnan",
  "overflow",
  "overflow_cast",
  "overflow_neg"
};

std::string
expr2t::pretty(unsigned int indent) const
{

  std::string ret = pretty_print_func<const expr2t&>(indent,
                                                     expr_names[expr_id],
                                                     *this);
  // Dump the type on the end.
  ret += std::string("\n") + indent_str(indent) + "type : "
         + type->pretty(indent + 2);
  return ret;
}

void
expr2t::dump(void) const
{
  std::cout << pretty(0) << std::endl;
  return;
}

/***************************** Templated expr body ****************************/

template <class derived>
expr_body<derived>::expr_body(const expr_body<derived> &ref)
  : expr2t(ref)
{
}

template <class derived>
void
expr_body<derived>::convert_smt(prop_convt &obj, void *&arg) const
{
  const derived *new_this = static_cast<const derived*>(this);
  obj.convert_smt_expr(*new_this, arg);
  return;
}

template <class derived>
expr2tc
expr_body<derived>::clone(void) const
{
  const derived *derived_this = static_cast<const derived*>(this);
  derived *new_obj = new derived(*derived_this);
  return expr2tc(new_obj);
}

/**************************** Expression constructors *************************/

unsigned long
constant_int2t::as_ulong(void) const
{
  // XXXjmorse - add assertion that we don't exceed machine word width?
  assert(!constant_value.is_negative());
  return constant_value.to_ulong();
}

long
constant_int2t::as_long(void) const
{
  // XXXjmorse - add assertion that we don't exceed machine word width?
  return constant_value.to_long();
}

bool
constant_bool2t::is_true(void) const
{
  return constant_value;
}

bool
constant_bool2t::is_false(void) const
{
  return !constant_value;
}

expr2tc
constant_string2t::to_array(void) const
{
  std::vector<expr2tc> contents;
  unsigned int length = value.size(), i;

  type2tc type = type_pool.get_uint8();

  for (i = 0; i < length; i++) {
    constant_int2t *v = new constant_int2t(type, BigInt(value[i]));
    expr2tc ptr(v);
    contents.push_back(ptr);
  }

  unsignedbv_type2t *len_type = new unsignedbv_type2t(config.ansi_c.int_width);
  type2tc len_tp(len_type);
  constant_int2t *len_val = new constant_int2t(len_tp, BigInt(length));
  expr2tc len_val_ref(len_val);

  array_type2t *arr_type = new array_type2t(type, len_val_ref, false);
  type2tc arr_tp(arr_type);
  constant_array2t *a = new constant_array2t(arr_tp, contents);

  expr2tc final_val(a);
  return final_val;
}

byte_extract2t::byte_extract2t(const type2tc type, bool is_big_endian,
                               const expr2tc source,
                               const expr2tc offs)
  : byte_ops2t<byte_extract2t>(type, byte_extract_id),
                                 big_endian(is_big_endian),
                                 source_value(source),
                                 source_offset(offs)
{
}

byte_extract2t::byte_extract2t(const byte_extract2t &ref)
  : byte_ops2t<byte_extract2t>(ref),
                              big_endian(ref.big_endian),
                              source_value(ref.source_value),
                              source_offset(ref.source_offset)
{
}

bool
byte_extract2t::cmp(const expr2t &ref) const
{
  const byte_extract2t &ref2 = static_cast<const byte_extract2t &>(ref);
 
  if (big_endian != ref2.big_endian)
    return false;

  if (source_value != ref2.source_value)
    return false;

  if (source_offset != ref2.source_offset)
    return false;

  return true;
}

int
byte_extract2t::lt(const expr2t &ref) const
{
  const byte_extract2t &ref2 = static_cast<const byte_extract2t &>(ref);

  if (big_endian < ref2.big_endian)
    return -1;
  if (big_endian > ref2.big_endian)
    return 1;

  int tmp = source_value->ltchecked(*ref2.source_value.get());
  if (tmp != 0)
    return tmp;

  return source_offset->ltchecked(*ref2.source_offset.get());
}

list_of_memberst
byte_extract2t::tostring(unsigned int indent) const
{
  list_of_memberst membs =
         tostring_func<expr2tc>(indent,
                                (const char *)"source_value", &source_value,
                                (const char *)"source_offset", &source_offset,
                                (const char *)"");
  membs.push_back(member_entryt("big_endian", (big_endian) ? "true" : "false"));
  return membs;
}

void
byte_extract2t::do_crc(boost::crc_32_type &crc) const
{
  expr2t::do_crc(crc);
  if (big_endian)
    crc.process_byte(1);
  else
    crc.process_byte(0);
  source_value->do_crc(crc);
  source_offset->do_crc(crc);
  return;
}

byte_update2t::byte_update2t(const type2tc type, bool is_big_endian,
                             const expr2tc source, const expr2tc offs,
                             const expr2tc update)
  : byte_ops2t<byte_update2t>(type, byte_update_id),
                                 big_endian(is_big_endian),
                                 source_value(source),
                                 source_offset(offs),
                                 update_value(update)
{
}

byte_update2t::byte_update2t(const byte_update2t &ref)
  : byte_ops2t<byte_update2t>(ref),
                              big_endian(ref.big_endian),
                              source_value(ref.source_value),
                              source_offset(ref.source_offset),
                              update_value(ref.update_value)
{
}

bool
byte_update2t::cmp(const expr2t &ref) const
{
  const byte_update2t &ref2 = static_cast<const byte_update2t &>(ref);
 
  if (big_endian != ref2.big_endian)
    return false;

  if (source_value != ref2.source_value)
    return false;

  if (source_offset != ref2.source_offset)
    return false;

  if (update_value != ref2.update_value)
    return false;

  return true;
}

int
byte_update2t::lt(const expr2t &ref) const
{
  const byte_update2t &ref2 = static_cast<const byte_update2t &>(ref);

  if (big_endian < ref2.big_endian)
    return -1;
  if (big_endian > ref2.big_endian)
    return 1;

  int tmp = source_value->ltchecked(*ref2.source_value.get());
  if (tmp != 0)
    return tmp;

  tmp = source_offset->ltchecked(*ref2.source_offset.get());
  if (tmp != 0)
    return tmp;

  return update_value->ltchecked(*ref2.update_value.get());
}

list_of_memberst
byte_update2t::tostring(unsigned int indent) const
{
  list_of_memberst membs =
         tostring_func<expr2tc>(indent,
                                (const char *)"source_value", &source_value,
                                (const char *)"source_offset", &source_offset,
                                (const char *)"update_value", &update_value,
                                (const char *)"");
  membs.push_back(member_entryt("big_endian", (big_endian) ? "true" : "false"));
  return membs;
}

void
byte_update2t::do_crc(boost::crc_32_type &crc) const
{
  expr2t::do_crc(crc);
  if (big_endian)
    crc.process_byte(1);
  else
    crc.process_byte(0);
  source_value->do_crc(crc);
  source_offset->do_crc(crc);
  update_value->do_crc(crc);
  return;
}

with2t::with2t(const type2tc type, const expr2tc source, const expr2tc idx,
               const expr2tc update)
  : datatype_ops2t<with2t>(type, with_id), source_data(source),
                          update_field(idx), update_data(update)
{
}

with2t::with2t(const with2t &ref)
  : datatype_ops2t<with2t>(ref), source_data(ref.source_data),
                         update_field(ref.update_field),
                         update_data(ref.update_data)
{
}

bool
with2t::cmp(const expr2t &ref) const
{
  const with2t &ref2 = static_cast<const with2t &>(ref);
 
  if (source_data != ref2.source_data)
    return false;

  if (update_field != ref2.update_field)
    return false;

  if (update_data != ref2.update_data)
    return false;

  return true;
}

int
with2t::lt(const expr2t &ref) const
{
  const with2t &ref2 = static_cast<const with2t &>(ref);

  int tmp = source_data->ltchecked(*ref2.source_data.get());
  if (tmp != 0)
    return tmp;

  tmp = update_field->ltchecked(*ref2.update_field.get());
  if (tmp != 0)
    return tmp;

  return update_data->ltchecked(*ref2.update_data.get());
}

list_of_memberst
with2t::tostring(unsigned int indent) const
{
  return tostring_func<expr2tc>(indent,
                                (const char *)"source_data", &source_data,
                                (const char *)"update_field", &update_field,
                                (const char *)"update_data", &update_data,
                                (const char *)"");
}

void
with2t::do_crc(boost::crc_32_type &crc) const
{
  expr2t::do_crc(crc);
  source_data->do_crc(crc);
  update_field->do_crc(crc);
  update_data->do_crc(crc);
  return;
}

member2t::member2t(const type2tc type, const expr2tc source,
                   const constant_string2t &idx)
  : datatype_ops2t<member2t>(type, member_id), source_data(source),
                          member(idx)
{
}

member2t::member2t(const member2t &ref)
  : datatype_ops2t<member2t>(ref), source_data(ref.source_data),
                           member(ref.member)
{
}

bool
member2t::cmp(const expr2t &ref) const
{
  const member2t &ref2 = static_cast<const member2t &>(ref);

  if (source_data != ref2.source_data)
    return false;

  if (member != ref2.member)
    return false;

  return true;
}

int
member2t::lt(const expr2t &ref) const
{
  const member2t &ref2 = static_cast<const member2t &>(ref);

  int tmp = source_data->ltchecked(*ref2.source_data.get());
  if (tmp != 0)
    return tmp;

  return member.ltchecked(ref2.member);
}

list_of_memberst
member2t::tostring(unsigned int indent) const
{
  list_of_memberst memb;

  memb.push_back(member_entryt("source", source_data->pretty(indent + 2)));
  memb.push_back(member_entryt("member name", member.value));
  return memb;
}

void
member2t::do_crc(boost::crc_32_type &crc) const
{
  expr2t::do_crc(crc);
  source_data->do_crc(crc);
  member.do_crc(crc);
  return;
}

index2t::index2t(const type2tc type, const expr2tc source,
                 const expr2tc _index)
  : datatype_ops2t<index2t>(type, index_id), source_data(source),
                           index(_index)
{
}

index2t::index2t(const index2t &ref)
  : datatype_ops2t<index2t>(ref), source_data(ref.source_data),
                           index(ref.index)
{
}

bool
index2t::cmp(const expr2t &ref) const
{
  const index2t &ref2 = static_cast<const index2t &>(ref);

  if (source_data != ref2.source_data)
    return false;

  if (index != ref2.index)
    return false;

  return true;
}

int
index2t::lt(const expr2t &ref) const
{
  const index2t &ref2 = static_cast<const index2t &>(ref);

  int tmp = source_data->ltchecked(*ref2.source_data.get());
  if (tmp != 0)
    return tmp;

  return index->ltchecked(*ref2.index.get());
}

list_of_memberst
index2t::tostring(unsigned int indent) const
{
  return tostring_func<expr2tc>(indent,
                                (const char *)"source_data", &source_data,
                                (const char *)"index", &index,
                                (const char *)"");
}

void
index2t::do_crc(boost::crc_32_type &crc) const
{
  expr2t::do_crc(crc);
  source_data->do_crc(crc);
  index->do_crc(crc);
  return;
}

zero_string2t::zero_string2t(const expr2tc _string)
  : datatype_ops2t<zero_string2t>(type_pool.get_bool(), zero_string_id),
                                string(_string)
{
}

zero_string2t::zero_string2t(const zero_string2t &ref)
  : datatype_ops2t<zero_string2t>(ref), string(ref.string)
{
}

bool
zero_string2t::cmp(const expr2t &ref) const
{
  const zero_string2t &ref2 = static_cast<const zero_string2t &>(ref);
  return string == ref2.string;
}

int
zero_string2t::lt(const expr2t &ref) const
{
  const zero_string2t &ref2 = static_cast<const zero_string2t &>(ref);
  return string->ltchecked(*ref2.string.get());
}

list_of_memberst
zero_string2t::tostring(unsigned int indent) const
{
  return tostring_func<expr2tc>(indent,
                                (const char *)"string", &string,
                                (const char *)"");
}

void
zero_string2t::do_crc(boost::crc_32_type &crc) const
{
  expr2t::do_crc(crc);
  string->do_crc(crc);
  return;
}

zero_length_string2t::zero_length_string2t(const expr2tc _string)
  : datatype_ops2t<zero_length_string2t>(type_pool.get_bool(),
                                        zero_length_string_id),
                                        string(_string)
{
}

zero_length_string2t::zero_length_string2t(const zero_length_string2t &ref)
  : datatype_ops2t<zero_length_string2t>(ref), string(ref.string)
{
}

bool
zero_length_string2t::cmp(const expr2t &ref) const
{
  const zero_length_string2t &ref2 = static_cast<const zero_length_string2t &>
                                                (ref);
  return string == ref2.string;
}

int
zero_length_string2t::lt(const expr2t &ref) const
{
  const zero_length_string2t &ref2 = static_cast<const zero_length_string2t &>
                                                (ref);
  return string->ltchecked(*ref2.string.get());
}

list_of_memberst
zero_length_string2t::tostring(unsigned int indent) const
{
  return tostring_func<expr2tc>(indent,
                                (const char *)"string", &string,
                                (const char *)"");
}

void
zero_length_string2t::do_crc(boost::crc_32_type &crc) const
{
  expr2t::do_crc(crc);
  string->do_crc(crc);
  return;
}

isnan2t::isnan2t(const expr2tc val)
  : lops2t<isnan2t>(isnan_id), value(val)
{
}

isnan2t::isnan2t(const isnan2t &ref)
  : lops2t<isnan2t>(ref), value(ref.value)
{
}

bool
isnan2t::cmp(const expr2t &ref) const
{
  const isnan2t &ref2 = static_cast<const isnan2t &> (ref);
  return value == ref2.value;
}

int
isnan2t::lt(const expr2t &ref) const
{
  const isnan2t &ref2 = static_cast<const isnan2t &> (ref);
  return value->ltchecked(*ref2.value.get());
}

list_of_memberst
isnan2t::tostring(unsigned int indent) const
{
  return tostring_func<expr2tc>(indent,
                                (const char *)"value", &value,
                                (const char *)"");
}

void
isnan2t::do_crc(boost::crc_32_type &crc) const
{
  expr2t::do_crc(crc);
  value->do_crc(crc);
  return;
}

overflow2t::overflow2t(const expr2tc val)
  : lops2t<overflow2t>(overflow_id), operand(val)
{
  assert((operand->expr_id == add_id || operand->expr_id == sub_id ||
          operand->expr_id == mul_id) && "operand to overflow2t must be "
          "add, sub or mul");
}

overflow2t::overflow2t(const overflow2t &ref)
  : lops2t<overflow2t>(ref), operand(ref.operand)
{
}

bool
overflow2t::cmp(const expr2t &ref) const
{
  const overflow2t &ref2 = static_cast<const overflow2t &> (ref);
  return operand == ref2.operand;
}

int
overflow2t::lt(const expr2t &ref) const
{
  const overflow2t &ref2 = static_cast<const overflow2t &> (ref);
  return operand->ltchecked(*ref2.operand.get());
}

list_of_memberst
overflow2t::tostring(unsigned int indent) const
{
  return tostring_func<expr2tc>(indent,
                                (const char *)"operand", &operand,
                                (const char *)"");
}

void
overflow2t::do_crc(boost::crc_32_type &crc) const
{
  expr2t::do_crc(crc);
  operand->do_crc(crc);
  return;
}

overflow_cast2t::overflow_cast2t(const expr2tc val, unsigned int _bits)
  : lops2t<overflow_cast2t>(overflow_cast_id), operand(val), bits(_bits)
{
}

overflow_cast2t::overflow_cast2t(const overflow_cast2t &ref)
  : lops2t<overflow_cast2t>(ref), operand(ref.operand), bits(ref.bits)
{
}

bool
overflow_cast2t::cmp(const expr2t &ref) const
{
  const overflow_cast2t &ref2 = static_cast<const overflow_cast2t &> (ref);
  if (bits != ref2.bits)
    return false;
  return operand == ref2.operand;
}

int
overflow_cast2t::lt(const expr2t &ref) const
{
  const overflow_cast2t &ref2 = static_cast<const overflow_cast2t &> (ref);
  if (bits < ref2.bits)
    return -1;
  else if (bits > ref2.bits)
    return 1;

  return operand->ltchecked(*ref2.operand.get());
}

list_of_memberst
overflow_cast2t::tostring(unsigned int indent) const
{
  list_of_memberst membs = tostring_func<expr2tc>(indent,
                                (const char *)"operand", &operand,
                                (const char *)"");
  char buffer[64];
  snprintf(buffer, 63, "%d", bits);
  buffer[63] = '\0';
  membs.push_back(member_entryt("width", std::string(buffer)));
  return membs;
}

void
overflow_cast2t::do_crc(boost::crc_32_type &crc) const
{
  expr2t::do_crc(crc);
  operand->do_crc(crc);
  crc.process_bytes(&bits, sizeof(bits));
  return;
}


overflow_neg2t::overflow_neg2t(const expr2tc val)
  : lops2t<overflow_neg2t>(overflow_neg_id), operand(val)
{
}

overflow_neg2t::overflow_neg2t(const overflow_neg2t &ref)
  : lops2t<overflow_neg2t>(ref), operand(ref.operand)
{
}

bool
overflow_neg2t::cmp(const expr2t &ref) const
{
  const overflow_neg2t &ref2 = static_cast<const overflow_neg2t &> (ref);
  return operand == ref2.operand;
}

int
overflow_neg2t::lt(const expr2t &ref) const
{
  const overflow_neg2t &ref2 = static_cast<const overflow_neg2t &> (ref);
  return operand->ltchecked(*ref2.operand.get());
}

list_of_memberst
overflow_neg2t::tostring(unsigned int indent) const
{
  return tostring_func<expr2tc>(indent,
                                (const char *)"operand", &operand,
                                (const char *)"");
}

void
overflow_neg2t::do_crc(boost::crc_32_type &crc) const
{
  expr2t::do_crc(crc);
  operand->do_crc(crc);
  return;
}

type_poolt::type_poolt(void)
{
  bool_type = type2tc(new bool_type2t());
  empty_type = type2tc(new empty_type2t());
  code_type = type2tc(new code_type2t());

  // Create some int types.
  type2tc ubv8(new unsignedbv_type2t(8));
  type2tc ubv16(new unsignedbv_type2t(16));
  type2tc ubv32(new unsignedbv_type2t(32));
  type2tc ubv64(new unsignedbv_type2t(64));
  type2tc sbv8(new signedbv_type2t(8));
  type2tc sbv16(new signedbv_type2t(16));
  type2tc sbv32(new signedbv_type2t(32));
  type2tc sbv64(new signedbv_type2t(64));

  unsignedbv_map[unsignedbv_typet(8)] = ubv8;
  unsignedbv_map[unsignedbv_typet(16)] = ubv16;
  unsignedbv_map[unsignedbv_typet(32)] = ubv32;
  unsignedbv_map[unsignedbv_typet(64)] = ubv64;
  signedbv_map[signedbv_typet(8)] = sbv8;
  signedbv_map[signedbv_typet(16)] = sbv16;
  signedbv_map[signedbv_typet(32)] = sbv32;
  signedbv_map[signedbv_typet(64)] = sbv64;

  uint8 = &unsignedbv_map[unsignedbv_typet(8)];
  uint16 = &unsignedbv_map[unsignedbv_typet(16)];
  uint32 = &unsignedbv_map[unsignedbv_typet(32)];
  uint64 = &unsignedbv_map[unsignedbv_typet(64)];
  int8 = &signedbv_map[signedbv_typet(8)];
  int16 = &signedbv_map[signedbv_typet(16)];
  int32 = &signedbv_map[signedbv_typet(32)];
  int64 = &signedbv_map[signedbv_typet(64)];

  return;
}

static const type2tc &
get_type_from_pool(const typet &val, std::map<const typet, type2tc> &map)
{
  std::map<const typet, type2tc>::const_iterator it = map.find(val);
  if (it != map.end())
    return it->second;

  type2tc new_type;
  real_migrate_type(val, new_type);
  map[val] = new_type;
  return map[val];
}

const type2tc &
type_poolt::get_struct(const typet &val)
{
  return get_type_from_pool(val, struct_map);
}

const type2tc &
type_poolt::get_union(const typet &val)
{
  return get_type_from_pool(val, union_map);
}

const type2tc &
type_poolt::get_array(const typet &val)
{
  return get_type_from_pool(val, array_map);
}

const type2tc &
type_poolt::get_pointer(const typet &val)
{
  return get_type_from_pool(val, pointer_map);
}

const type2tc &
type_poolt::get_unsignedbv(const typet &val)
{
  return get_type_from_pool(val, unsignedbv_map);
}

const type2tc &
type_poolt::get_signedbv(const typet &val)
{
  return get_type_from_pool(val, signedbv_map);
}

const type2tc &
type_poolt::get_fixedbv(const typet &val)
{
  return get_type_from_pool(val, fixedbv_map);
}

const type2tc &
type_poolt::get_string(const typet &val)
{
  return get_type_from_pool(val, string_map);
}

const type2tc &
type_poolt::get_symbol(const typet &val)
{
  return get_type_from_pool(val, symbol_map);
}

const type2tc &
type_poolt::get_uint(unsigned int size)
{
  switch (size) {
  case 8:
    return get_uint8();
  case 16:
    return get_uint16();
  case 32:
    return get_uint32();
  case 64:
    return get_uint64();
  default:
    return get_unsignedbv(unsignedbv_typet(size));
  }
}

const type2tc &
type_poolt::get_int(unsigned int size)
{
  switch (size) {
  case 8:
    return get_int8();
  case 16:
    return get_int16();
  case 32:
    return get_int32();
  case 64:
    return get_int64();
  default:
    return get_signedbv(signedbv_typet(size));
  }
}

type_poolt type_pool;

// For CRCing to actually be accurate, expr/type ids mustn't overflow out of
// a byte. If this happens then a) there are too many exprs, and b) the expr
// crcing code has to change.
BOOST_STATIC_ASSERT(type2t::end_type_id <= 256);
BOOST_STATIC_ASSERT(expr2t::end_expr_id <= 256);

template <>
inline std::string
type_to_string<bool>(const bool &thebool, int indent __attribute__((unused)))
{
  return (thebool) ? "true" : "false";
}

template <>
inline std::string
type_to_string<BigInt>(const BigInt &theint, int indent __attribute__((unused)))
{
  char buffer[256], *buf;

  buf = theint.as_string(buffer, 256);
  return std::string(buf);
}

template <>
inline std::string
type_to_string<fixedbvt>(const fixedbvt &theval,
                         int indent __attribute__((unused)))
{
  return theval.to_ansi_c_string();
}

template <>
inline std::string
type_to_string<std::string>(const std::string &theval,
                         int indent __attribute__((unused)))
{
  return theval;
}

template <>
inline std::string
type_to_string<std::vector<expr2tc> >(const std::vector<expr2tc> &theval,
                                     int indent)
{
  char buffer[64];
  std::string astring = "";
  int i;

  i = 0;
  forall_exprs(it, theval) {
    snprintf(buffer, 63, "%d", i);
    buffer[63] = '\0';
    astring += std::string(buffer) + ": " + (*it)->pretty(indent + 2) + "\n";
    i++;
  }

  return astring;
}

template <>
inline std::string
type_to_string<expr2tc>(const expr2tc &theval, int indent)
{

  return theval->pretty(indent + 2);
}

template <>
inline std::string
type_to_string<irep_idt>(const irep_idt &theval,
                         int indent __attribute__((unused)))
{
  return theval.as_string();
}

template <>
inline bool
do_type_cmp<bool>(const bool &side1, const bool &side2)
{
  return (side1 == side2) ? true : false;
}

template <>
inline bool
do_type_cmp<BigInt>(const BigInt &side1, const BigInt &side2)
{
  // BigInt has its own equality operator.
  return (side1 == side2) ? true : false;
}

template <>
inline bool
do_type_cmp<fixedbvt>(const fixedbvt &side1, const fixedbvt &side2)
{
  return (side1 == side2) ? true : false;
}

template <>
inline bool
do_type_cmp<std::string>(const std::string &side1, const std::string &side2)
{
  return (side1 == side2) ? true : false;
}

template <>
inline bool
do_type_cmp<std::vector<expr2tc> >(const std::vector<expr2tc> &side1,
                                   const std::vector<expr2tc> &side2)
{
  return (side1 == side2);
}

template <>
inline bool
do_type_cmp<expr2tc>(const expr2tc &side1, const expr2tc &side2)
{
  return (side1 == side2);
}

template <>
inline bool
do_type_cmp<irep_idt>(const irep_idt &side1, const irep_idt &side2)
{
  return (side1 == side2);
}

template <>
inline int
do_type_lt<bool>(const bool &side1, const bool &side2)
{
  if (side1 < side2)
    return -1;
  else if (side2 < side1)
    return 1;
  else
    return 0;
}

template <>
inline int
do_type_lt<BigInt>(const BigInt &side1, const BigInt &side2)
{
  // BigInt also has its own less than comparator.
  return side1.compare(side2);
}

template <>
inline int
do_type_lt<fixedbvt>(const fixedbvt &side1, const fixedbvt &side2)
{
  if (side1 < side2)
    return -1;
  else if (side1 > side2)
    return 1;
  return 0;
}

template <>
inline int
do_type_lt<std::string>(const std::string &side1, const std::string &side2)
{
  if (side1 < side2)
    return -1;
  else if (side2 < side1)
    return 1;
  return 0;
}

template <>
inline int
do_type_lt<std::vector<expr2tc> >(const std::vector<expr2tc> &side1,
                                  const std::vector<expr2tc> &side2)
{
  if (side1 < side2)
    return -1;
  else if (side2 < side1)
    return 1;
  return 0;
}

template <>
inline int
do_type_lt<expr2tc>(const expr2tc &side1, const expr2tc &side2)
{
  return side1->ltchecked(*side2.get());
}

template <>
inline int
do_type_lt<irep_idt>(const irep_idt &side1, const irep_idt &side2)
{
  if (side1 < side2)
    return -1;
  if (side2 < side1)
    return 1;
  return 0;
}

template <>
inline void
do_type_crc<bool>(const bool &thebool, boost::crc_32_type &crc)
{

  if (thebool)
    crc.process_byte(0);
  else
    crc.process_byte(1);
  return;
}

template <>
inline void
do_type_crc<BigInt>(const BigInt &theint, boost::crc_32_type &crc)
{
  unsigned char buffer[256];

  if (theint.dump(buffer, sizeof(buffer))) {
    // Zero has no data in bigints.
    if (theint.is_zero())
      crc.process_byte(0);
    else
      crc.process_bytes(buffer, theint.get_len());
  } else {
    // bigint is too large to fit in that static buffer. This is insane; but
    // rather than wasting time heap allocing we'll just skip recording data,
    // at the price of possible crc collisions.
    ;
  }
  return;
}

template <>
inline void
do_type_crc<fixedbvt>(const fixedbvt &theval, boost::crc_32_type &crc)
{

  do_type_crc<BigInt>(theval.to_integer(), crc);
  return;
}

template <>
inline void
do_type_crc<std::string>(const std::string &theval, boost::crc_32_type &crc)
{
  crc.process_bytes(theval.c_str(), theval.size());
}

template <>
inline void
do_type_crc<std::vector<expr2tc> >(const std::vector<expr2tc> &theval,
                                   boost::crc_32_type &crc)
{
  forall_exprs(it, theval)
    (*it)->do_crc(crc);
}

template <>
inline void
do_type_crc<expr2tc>(const expr2tc &theval, boost::crc_32_type &crc)
{

  theval->do_crc(crc);
  return;
}

template <>
inline void
do_type_crc<irep_idt>(const irep_idt &theval, boost::crc_32_type &crc)
{

  crc.process_bytes(theval.as_string().c_str(), theval.as_string().size());
  return;
}

template <class derived, class field1, class field2, class field3, class field4>
void
expr_body2<derived, field1, field2, field3, field4>::convert_smt(prop_convt &obj, void *&arg) const
{
  const derived *new_this = static_cast<const derived*>(this);
  obj.convert_smt_expr(*new_this, arg);
  return;
}

template <class derived, class field1, class field2, class field3, class field4>
expr2tc
expr_body2<derived, field1, field2, field3, field4>::clone(void) const
{
  const derived *derived_this = static_cast<const derived*>(this);
  derived *new_obj = new derived(*derived_this);
  return expr2tc(new_obj);
}

template <class derived, class field1, class field2, class field3, class field4>
list_of_memberst
expr_body2<derived, field1, field2, field3, field4>::tostring(unsigned int indent) const
{
  list_of_memberst thevector;
  field1::fieldtype::tostring(thevector, indent);
  field2::fieldtype::tostring(thevector, indent);
  field3::fieldtype::tostring(thevector, indent);
  field4::fieldtype::tostring(thevector, indent);
  return thevector;
}

template <class derived, class field1, class field2, class field3, class field4>
bool
expr_body2<derived, field1, field2, field3, field4>::cmp(const expr2t &ref)const
{
  const derived &ref2 = static_cast<const derived &>(ref);

  if (!field1::fieldtype::cmp(
        static_cast<const typename field1::fieldtype &>(ref2)))
    return false;

  if (!field2::fieldtype::cmp(
        static_cast<const typename field2::fieldtype &>(ref2)))
    return false;

  if (!field3::fieldtype::cmp(
        static_cast<const typename field3::fieldtype &>(ref2)))
    return false;

  if (!field4::fieldtype::cmp(
        static_cast<const typename field4::fieldtype &>(ref2)))
    return false;

  return true;
}

template <class derived, class field1, class field2, class field3, class field4>
int
expr_body2<derived, field1, field2, field3, field4>::lt(const expr2t &ref)const
{
  int tmp;
  const derived &ref2 = static_cast<const derived &>(ref);

  tmp = field1::fieldtype::lt(
                static_cast<const typename field1::fieldtype &>(ref2));
  if (tmp != 0)
    return tmp;

  tmp = field2::fieldtype::lt(
                static_cast<const typename field2::fieldtype &>(ref2));
  if (tmp != 0)
    return tmp;

  tmp = field3::fieldtype::lt(
                static_cast<const typename field3::fieldtype &>(ref2));
  if (tmp != 0)
    return tmp;

  tmp = field4::fieldtype::lt(
                static_cast<const typename field4::fieldtype &>(ref2));

  return tmp;
}

template <class derived, class field1, class field2, class field3, class field4>
void
expr_body2<derived, field1, field2, field3, field4>::do_crc
          (boost::crc_32_type &crc) const
{

  expr2t::do_crc(crc);
  field1::fieldtype::do_crc(crc);
  field2::fieldtype::do_crc(crc);
  field3::fieldtype::do_crc(crc);
  field4::fieldtype::do_crc(crc);
  return;
}
