
#ifndef OPENDDS_DCPS_XTYPES_TYPE_DESCRIPTOR_H
#define OPENDDS_DCPS_XTYPES_TYPE_DESCRIPTOR_H

#include "External.h"

#include <dds/DCPS/XTypes/DynamicType.h>

OPENDDS_BEGIN_VERSIONED_NAMESPACE_DECL
namespace OpenDDS {
namespace XTypes {

enum ExtensibilityKind {
 FINAL,
 APPENDABLE,
 MUTABLE
};

class TypeDescriptor {
 public:
  TypeDescriptor()
  : kind(0), extensibility_kind(FINAL), is_nested(0)
  {
  }
  TypeKind kind;
  OPENDDS_STRING name;
  DynamicType_rch base_type;
  DynamicType_rch discriminator_type;
  LBoundSeq bound;
  DynamicType_rch element_type;
  DynamicType_rch key_element_type;
  ExtensibilityKind extensibility_kind;
  bool is_nested;
  bool equals(const TypeDescriptor& other);
 };

inline bool operator==(const LBoundSeq& lhs, const LBoundSeq& rhs)
{
  if (lhs.length() == rhs.length()) {
    for (ulong i = 0 ; i < lhs.length() ; ++i) {
      if (!(lhs[i] == rhs[i])) {
        return false;
      }
    }
  } else {
    return false;
  }
  return true;
}

inline bool operator==(const TypeDescriptor& lhs, const TypeDescriptor& rhs)
{
  bool a = lhs.kind == rhs.kind;
  bool b = lhs.name == rhs.name;
  bool c = is_equivalent(lhs.base_type, rhs.base_type);
  bool d = is_equivalent(lhs.discriminator_type, rhs.discriminator_type);
  bool e = lhs.bound == rhs.bound;
  bool f = is_equivalent(lhs.element_type, rhs.element_type);
  bool g = is_equivalent(lhs.key_element_type, rhs.key_element_type);
  bool h = lhs.extensibility_kind == rhs.extensibility_kind;
  bool i = lhs.is_nested == rhs.is_nested;
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("TypeDescriptor: %b %b %b %b %b %b %b %b %b\n"),
  a, b, c, d, e, f, g, h, i));
  return a && b && c && d && e && f && g && h && i;
  // return
  //   lhs.kind == rhs.kind &&
  //   lhs.name == rhs.name &&
  //   lhs.base_type == rhs.base_type &&
  //   lhs.discriminator_type == rhs.discriminator_type &&
  //   lhs.bound == rhs.bound &&
  //   lhs.element_type.in() == rhs.element_type.in() &&
  //   lhs.key_element_type.in() == rhs.key_element_type.in() &&
  //   lhs.extensibility_kind == rhs.extensibility_kind &&
  //   lhs.is_nested == rhs.is_nested;
}

} // namespace XTypes
} // namespace OpenDDS

OPENDDS_END_VERSIONED_NAMESPACE_DECL

#endif  /* OPENDDS_DCPS_XTYPES_TYPE_DESCRIPTOR_H */