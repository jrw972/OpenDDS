
#ifndef OPENDDS_DCPS_XTYPES_MEMBER_DESCRIPTOR_H
#define OPENDDS_DCPS_XTYPES_MEMBER_DESCRIPTOR_H

#include "External.h"

#include <dds/DCPS/XTypes/DynamicType.h>

OPENDDS_BEGIN_VERSIONED_NAMESPACE_DECL
namespace OpenDDS {
namespace XTypes {

enum TryConstructKind {
 USE_DEFAULT,
 DISCARD,
 TRIM
};

class MemberDescriptor {
public:
  MemberDescriptor()
  : id(0), index(0), try_construct_kind(DISCARD), is_key(0), is_optional(0), is_must_understand(0), is_shared(0), is_default_label(0)
  {
  }
  OPENDDS_STRING name;
  XTypes::MemberId id;
  DynamicType_rch type;
  OPENDDS_STRING default_value;
  unsigned long index;
  XTypes::UnionCaseLabelSeq label;
  TryConstructKind try_construct_kind;
  bool is_key;
  bool is_optional;
  bool is_must_understand;
  bool is_shared;
  bool is_default_label;
  bool equals(const MemberDescriptor& descriptor);
 };

inline bool operator==(const UnionCaseLabelSeq& lhs, const UnionCaseLabelSeq& rhs)
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

inline bool operator==(const MemberDescriptor& lhs, const MemberDescriptor& rhs)
{
  //TODO CLAYTON : Uncomment and delete the code used for debugging upon completion of test
  bool a = lhs.name == rhs.name;
  bool b =  lhs.id == rhs.id;
  bool c =  is_equivalent(lhs.type, rhs.type);
  bool d =  lhs.default_value == rhs.default_value;
  bool e =  lhs.index == rhs.index;
  bool f =  lhs.label == rhs.label;
  bool g =  lhs.try_construct_kind == rhs.try_construct_kind;
  bool h =  lhs.is_key == rhs.is_key;
  bool i =  lhs.is_optional == rhs.is_optional;
  bool j =  lhs.is_must_understand == rhs.is_must_understand;
  bool k =  lhs.is_shared == rhs.is_shared;
  bool l =  lhs.is_default_label == rhs.is_default_label;
  ACE_DEBUG((LM_DEBUG, ACE_TEXT("MemberDescriptor: %b %b %b %b %b %b %b %b %b %b %b %b\n"),
  a, b, c, d, e, f, g, h, i, j, k, l));
  return a && b && c && d && e && f && g && h && i && j && k && l;
  // return
  //   lhs.name == rhs.name &&
  //   lhs.id == rhs.id &&
  //   lhs.type == rhs.type &&
  //   lhs.default_value == rhs.default_value &&
  //   lhs.index == rhs.index &&
  //   lhs.label == rhs.label &&
  //   lhs.try_construct_kind == rhs.try_construct_kind &&
  //   lhs.is_key == rhs.is_key &&
  //   lhs.is_optional == rhs.is_optional &&
  //   lhs.is_must_understand == rhs.is_must_understand &&
  //   lhs.is_shared == rhs.is_shared &&
  //   lhs.is_default_label == rhs.is_default_label;
}

} // namespace XTypes
} // namespace OpenDDS

OPENDDS_END_VERSIONED_NAMESPACE_DECL

#endif  /* OPENDDS_DCPS_XTYPES_MEMBER_DESCRIPTOR_H */