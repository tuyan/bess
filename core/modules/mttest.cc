#include "mttest.h"

const Commands<Module> MetadataTest::cmds = {};

pb_error_t MetadataTest::AddAttributes(
    const google::protobuf::Map<std::string, int64_t> &attrs,
    bess::metadata::AccessMode mode) {
  for (const auto &kv : attrs) {
    int ret;

    const char *attr_name = kv.first.c_str();
    int attr_size = kv.second;

    ret = AddMetadataAttr(attr_name, attr_size, mode);
    if (ret < 0)
      return pb_error(-ret, "invalid metadata declaration");

    /* check /var/log/syslog for log messages */
    switch (mode) {
      case bess::metadata::AccessMode::READ:
        log_info("module %s: %s, %d bytes, read\n", name().c_str(), attr_name,
                 attr_size);
        break;
      case bess::metadata::AccessMode::WRITE:
        log_info("module %s: %s, %d bytes, write\n", name().c_str(), attr_name,
                 attr_size);
        break;
      case bess::metadata::AccessMode::UPDATE:
        log_info("module %s: %s, %d bytes, update\n", name().c_str(), attr_name,
                 attr_size);
        break;
    }
  }

  return pb_errno(0);
}

struct snobj *MetadataTest::AddAttributes(
    struct snobj *attrs, bess::metadata::AccessMode mode) {
  if (snobj_type(attrs) != TYPE_MAP) {
    return snobj_err(EINVAL,
                     "argument must be a map of "
                     "{'attribute name': size, ...}");
  }

  /* a bit hacky, since there is no iterator for maps... */
  for (size_t i = 0; i < attrs->size; i++) {
    int ret;

    const char *attr_name = attrs->map.arr_k[i];
    int attr_size = snobj_int_get((attrs->map.arr_v[i]));

    ret = AddMetadataAttr(attr_name, attr_size, mode);
    if (ret < 0)
      return snobj_err(-ret, "invalid metadata declaration");

    /* check /var/log/syslog for log messages */
    switch (mode) {
      case bess::metadata::AccessMode::READ:
        log_info("module %s: %s, %d bytes, read\n", name().c_str(), attr_name,
                 attr_size);
        break;
      case bess::metadata::AccessMode::WRITE:
        log_info("module %s: %s, %d bytes, write\n", name().c_str(), attr_name,
                 attr_size);
        break;
      case bess::metadata::AccessMode::UPDATE:
        log_info("module %s: %s, %d bytes, update\n", name().c_str(), attr_name,
                 attr_size);
        break;
    }
  }

  return nullptr;
}

pb_error_t MetadataTest::Init(const bess::protobuf::MetadataTestArg &arg) {
  pb_error_t err;

  err = AddAttributes(arg.read(), bess::metadata::AccessMode::READ);
  if (err.err() != 0) {
    return err;
  }

  err = AddAttributes(arg.write(), bess::metadata::AccessMode::WRITE);
  if (err.err() != 0) {
    return err;
  }

  err = AddAttributes(arg.update(), bess::metadata::AccessMode::UPDATE);
  if (err.err() != 0) {
    return err;
  }

  return pb_errno(0);
}

struct snobj *MetadataTest::Init(struct snobj *arg) {
  struct snobj *attrs;
  struct snobj *err;

  if ((attrs = snobj_eval(arg, "read"))) {
    err = AddAttributes(attrs, bess::metadata::AccessMode::READ);
    if (err) {
      return err;
    }
  }

  if ((attrs = snobj_eval(arg, "write"))) {
    err = AddAttributes(attrs, bess::metadata::AccessMode::WRITE);
    if (err) {
      return err;
    }
  }

  if ((attrs = snobj_eval(arg, "update"))) {
    err = AddAttributes(attrs, bess::metadata::AccessMode::UPDATE);
    if (err) {
      return err;
    }
  }

  return nullptr;
}

void MetadataTest::ProcessBatch(struct pkt_batch *batch) {
  /* This module simply passes packets from input gate X down
   * to output gate X (the same gate index) */
  RunChooseModule(get_igate(), batch);
}

ADD_MODULE(MetadataTest, "mt_test", "Dynamic metadata test module")