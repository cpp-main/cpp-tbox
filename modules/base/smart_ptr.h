#ifndef TBOX_SMART_PTR_H_20230402
#define TBOX_SMART_PTR_H_20230402

#include <memory>
#include <utility>

/**
 * Defines aliases and static functions for using the Class with smart pointers.
 *
 * Use in the public section of the class.
 */
#define TBOX_SMART_PTR_DEFINITIONS(...)                                     \
    __TBOX_SHARED_PTR_ALIAS(__VA_ARGS__)                                    \
    __TBOX_WEAK_PTR_ALIAS(__VA_ARGS__)                                      \
    __TBOX_UNIQUE_PTR_ALIAS(__VA_ARGS__)                                    \
    __TBOX_MAKE_SHARED_DEFINITION(__VA_ARGS__)                              \
    __TBOX_MAKE_UNIQUE_DEFINITION(__VA_ARGS__)

/**
 * Defines aliases and static functions for using the Class with smart pointers.
 *
 * Same as TBOX_SMART_PTR_DEFINITIONS except it excludes the static
 * Class::make_unique() method definition which does not work on classes which
 * are not CopyConstructable.
 *
 * Use in the public section of the class.
 */
#define TBOX_SMART_PTR_DEFINITIONS_NOT_COPYABLE(...)                        \
    __TBOX_SHARED_PTR_ALIAS(__VA_ARGS__)                                    \
    __TBOX_WEAK_PTR_ALIAS(__VA_ARGS__)                                      \
    __TBOX_UNIQUE_PTR_ALIAS(__VA_ARGS__)                                    \
    __TBOX_MAKE_SHARED_DEFINITION(__VA_ARGS__)                              \

/**
 * Defines aliases only for using the Class with smart pointers.
 *
 * Same as TBOX_SMART_PTR_DEFINITIONS except it excludes the static
 * method definitions which do not work on pure virtual classes and classes
 * which are not CopyConstructable.
 *
 * Use in the public section of the class.
 */
#define TBOX_SMART_PTR_ALIASES_ONLY(...)                                    \
    __TBOX_SHARED_PTR_ALIAS(__VA_ARGS__)                                    \
    __TBOX_WEAK_PTR_ALIAS(__VA_ARGS__)                                      \
    __TBOX_UNIQUE_PTR_ALIAS(__VA_ARGS__)                                    \
    __TBOX_MAKE_SHARED_DEFINITION(__VA_ARGS__)

#define __TBOX_SHARED_PTR_ALIAS(...)                                        \
    using SharedPtr = std::shared_ptr<__VA_ARGS__>;                         \
    using ConstSharedPtr = std::shared_ptr<const __VA_ARGS__>;

#define __TBOX_MAKE_SHARED_DEFINITION(...)                                  \
    template <typename... Args>                                             \
    static std::shared_ptr<__VA_ARGS__> MakeShared(Args &&...args) {       \
        return std::make_shared<__VA_ARGS__>(std::forward<Args>(args)...);  \
    }

#define __TBOX_WEAK_PTR_ALIAS(...)                                          \
    using WeakPtr = std::weak_ptr<__VA_ARGS__>;                             \
    using ConstWeakPtr = std::weak_ptr<const __VA_ARGS__>;

#define __TBOX_UNIQUE_PTR_ALIAS(...)                                        \
    using UniquePtr = std::unique_ptr<__VA_ARGS__>;

#define __TBOX_MAKE_UNIQUE_DEFINITION(...)                                  \
    template <typename... Args>                                             \
    static std::unique_ptr<__VA_ARGS__> MakeUnique(Args &&...args) {       \
        return std::unique_ptr<__VA_ARGS__>(                                \
                new __VA_ARGS__(std::forward<Args>(args)...));              \
    }

#endif // TBOX_SMART_PTR_H_20230402
