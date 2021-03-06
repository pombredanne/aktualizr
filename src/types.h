#ifndef TYPES_H_
#define TYPES_H_

#include <json/json.h>
#include <boost/filesystem.hpp>

enum KeyType { kED25519 = 0, kRSA2048, kRSA4096 };

namespace data {

typedef std::string UpdateRequestId;
struct Package {
  std::string name;
  std::string version;
  Json::Value toJson();
  static Package fromJson(const std::string&);
};

enum UpdateRequestStatus { Pending = 0, InFlight };

struct UpdateAvailable {
  std::string update_id;
  std::string signature;
  std::string description;
  bool request_confirmation;
  unsigned long long size;
  Json::Value toJson();
  static UpdateAvailable fromJson(const std::string& json_str);
  static UpdateAvailable fromJson(const Json::Value& json);
};

struct DownloadComplete {
  std::string update_id;
  std::string update_image;
  std::string signature;
  Json::Value toJson();
  static DownloadComplete fromJson(const std::string& json_str);
};

enum UpdateResultCode {
  /// Operation executed successfully
  OK = 0,
  /// Operation has already been processed
  ALREADY_PROCESSED,
  /// Dependency failure during package install, upgrade, or removal
  DEPENDENCY_FAILURE,
  /// Update image integrity has been compromised
  VALIDATION_FAILED,
  /// Package installation failed
  INSTALL_FAILED,
  /// Package upgrade failed
  UPGRADE_FAILED,
  /// Package removal failed
  REMOVAL_FAILED,
  /// The module loader could not flash its managed module
  FLASH_FAILED,
  /// Partition creation failed
  CREATE_PARTITION_FAILED,
  /// Partition deletion failed
  DELETE_PARTITION_FAILED,
  /// Partition resize failed
  RESIZE_PARTITION_FAILED,
  /// Partition write failed
  WRITE_PARTITION_FAILED,
  /// Partition patching failed
  PATCH_PARTITION_FAILED,
  /// User declined the update
  USER_DECLINED,
  /// Software was blacklisted
  SOFTWARE_BLACKLISTED,
  /// Ran out of disk space
  DISK_FULL,
  /// Software package not found
  NOT_FOUND,
  /// Tried to downgrade to older version
  OLD_VERSION,
  /// SWM Internal integrity error
  INTERNAL_ERROR,
  /// Other error
  GENERAL_ERROR
};

typedef std::pair<UpdateResultCode, std::string> InstallOutcome;

struct UpdateReport;
struct OperationResult {
  std::string id;
  UpdateResultCode result_code;
  std::string result_text;
  Json::Value toJson();
  UpdateReport toReport();
  bool isSuccess() { return result_code == OK || result_code == ALREADY_PROCESSED; };
  static OperationResult fromJson(const std::string& json_str);
  static OperationResult fromOutcome(const std::string& id, const InstallOutcome& outcome);
};

struct UpdateReport {
  UpdateRequestId update_id;
  std::vector<data::OperationResult> operation_results;
  Json::Value toJson();
  static UpdateReport fromJson(const std::string& json_str);
};
}

#endif
