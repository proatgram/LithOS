#pragma once

#include "Package.hpp"
#include "Transaction.hpp"
#include "Dependency.hpp"

namespace ALPM {
    class Events {
        public:
            enum class Type {

            };

            struct PackageOperationEvent {
                Package NewPackage;
                Package OldPackage;
                Transaction::PackageOperation Operation;
                Type EventType;
            };
            struct OptionalDependencyRemovalEvent {
                Dependency OptionalDependency;
                Package ContainingPackage;
                Type EventType;
            };
            struct ScriptletInfoEvent {
                std::string Output;
                Type EventType;
            };
            struct DatabaseMissingEvent {
                std::string Name;
                Type EventType;
            };
            struct PackageDownloadedEvent {
                std::string Filename;
                Type EventType;
            };

        private:
    };
}
