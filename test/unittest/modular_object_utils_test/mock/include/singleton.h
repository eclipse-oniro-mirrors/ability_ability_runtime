/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MOCK_SINGLETON_H
#define MOCK_SINGLETON_H

#include <memory>
#include <mutex>

template<typename T>
class DelayedSingleton {
public:
    static std::shared_ptr<T> GetInstance()
    {
        static std::once_flag onceFlag;
        static std::shared_ptr<T> instance;
        std::call_once(onceFlag, []() { instance = std::make_shared<T>(); });
        return instance;
    }
};

#define DECLARE_DELAYED_SINGLETON(cls) \
    friend class DelayedSingleton<cls>

#endif // MOCK_SINGLETON_H
