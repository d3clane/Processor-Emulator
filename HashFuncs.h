#ifndef HASH_FUNCS_H
#define HASH_FUNCS_H

/// @file
/// @brief Contains functions to hash

/// @brief HashType returned by functions
typedef uint64_t HashType;

/// @brief Calculates hash using murmur algo
/// @param [in]hashingArr array to hash
/// @param [in]length length of the array to hash in bytes
/// @param [in]seed seed for hashing. 0 by default
/// @return hash
HashType MurmurHash(const void* hashingArr, const size_t length, const uint64_t seed = 0);

#endif // HASH_FUNCS_H