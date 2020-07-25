// Tencent is pleased to support the open source community by making RapidJSON available.
// 
// Copyright (C) 2015 THL A29 Limited, a Tencent company, and Milo Yip. All rights reserved.
//
// Licensed under the MIT License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// http://opensource.org/licenses/MIT
//
// Unless required by applicable law or agreed to in writing, software distributed 
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
// CONDITIONS OF ANY KIND, either express or implied. See the License for the 
// specific language governing permissions and limitations under the License.

#ifndef RAPIDJSON_PRETTYWRITER2_H_
#define RAPIDJSON_PRETTYWRITER2_H_

//#include "writer.h"
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <unordered_map>
#include <unordered_set>
#include <memory>
//#include "QtHash.h"
#include "JsonNode.h"
#include "JsonTree.h"
#include "Algorithm.h"
#include "TextFormat.h"
#include "Constants.h"
#include "JsonTreeResult.h"

#ifdef __GNUC__
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(effc++)
#endif

//RAPIDJSON_NAMESPACE_BEGIN
namespace SDV {
//
////! Combination of PrettyWriter2 format flags.
///*! \see PrettyWriter2::SetFormatOptions
// */
//enum PrettyFormatOptions {
//    kFormatDefault = 0,         //!< Default pretty formatting.
//    kFormatSingleLineArray = 1  //!< Format arrays on a single line.
//};

//! Writer with indentation and spacing.
/*!
    \tparam OutputStream Type of ouptut os.
    \tparam SourceEncoding Encoding of source string.
    \tparam TargetEncoding Encoding of output stream.
    \tparam StackAllocator Type of allocator for allocating memory of stack.
*/
template<typename OutputStream, typename SourceEncoding = rapidjson::UTF8<>, typename TargetEncoding = rapidjson::UTF8<>, typename StackAllocator = rapidjson::CrtAllocator, unsigned writeFlags = rapidjson::kWriteDefaultFlags>
class PrettyWriter2 : public rapidjson::Writer<OutputStream, SourceEncoding, TargetEncoding, StackAllocator, writeFlags> {
public:
    typedef rapidjson::Writer<OutputStream, SourceEncoding, TargetEncoding, StackAllocator> Base;
    typedef typename Base::Ch Ch;

    //! Constructor
    /*! \param os Output stream.
        \param allocator User supplied allocator. If it is null, it will create a private one.
        \param levelDepth Initial capacity of stack.
    */
    explicit PrettyWriter2(OutputStream& os,
                           size_t bufferCapacity,
                           const std::unordered_map<JsonNodeType, TextFormat>& formatMap/*,
                           StackAllocator* allocator = 0, size_t levelDepth = Base::kDefaultLevelDepth*/) :
//        Base(os, allocator, levelDepth),
        Base(os), indentChar_(' '), indentCharCount_(4), formatOptions_(rapidjson::kFormatDefault),
        m_formatMap{formatMap}, m_lastBufferSize{0}
    {
        m_result = std::make_unique<JsonTreeResult>();
    }
//
//    explicit PrettyWriter2(StackAllocator* allocator = 0, size_t levelDepth = Base::kDefaultLevelDepth) :
//        Base(allocator, levelDepth), indentChar_(' '), indentCharCount_(4) {}

    std::unique_ptr<JsonTreeResult>
    result()
    {
        assert(nullptr != m_result.get());
        assert(nullptr != m_result->jsonTree->rootNode);
        assert(m_parentNodeVec.empty());
        assert(m_currentLine.isEmpty() == false);
        internedTextSet.clear();

        m_result->jsonTextLineVec.push_back(m_currentLine);
        m_currentLine.clear();

        m_result->jsonTextLineVec.shrink_to_fit();
        m_result->jsonTree->lineIndex_To_NodeVec.shrink_to_fit();
        // Intentional: std::move() is required with std::unique_ptr.
        return std::move(m_result);
    }

    //! Set custom indentation.
    /*! \param indentChar       Character for indentation. Must be whitespace character (' ', '\\t', '\\n', '\\r').
        \param indentCharCount  Number of indent characters for each indentation level.
        \note The default indentation is 4 spaces.
    */
    PrettyWriter2& SetIndent(Ch indentChar, unsigned indentCharCount)
    {
        RAPIDJSON_ASSERT(indentChar == ' ' || indentChar == '\t' || indentChar == '\n' || indentChar == '\r');
        indentChar_ = indentChar;
        indentCharCount_ = indentCharCount;
        return *this;
    }

    //! Set pretty writer formatting options.
    /*! \param options Formatting options.
    */
    PrettyWriter2& SetFormatOptions(rapidjson::PrettyFormatOptions options)
    {
        formatOptions_ = options;
        return *this;
    }

    /*! @name Implementation of Handler
        \see Handler
    */
    //@{

//    bool Null()                 { PrettyPrefix(kNullType);   return Base::WriteNull(); }
    bool Null()
    {
        PrettyPrefix(rapidjson::kNullType);
        append_();
        bool r = Base::WriteNull();
        append_(JsonNodeType::Null);
        return r;
    }
//    bool Bool(bool b)           { PrettyPrefix(b ? kTrueType : kFalseType); return Base::WriteBool(b); }
    bool Bool(bool b)
    {
        PrettyPrefix(b ? rapidjson::kTrueType : rapidjson::kFalseType);
        append_();
        bool r = Base::WriteBool(b);
        append_(JsonNodeType::Bool);
        return r;
    }
//    bool Int(int i)             { PrettyPrefix(kNumberType); return Base::WriteInt(i); }
    bool Int(int i)
    {
        PrettyPrefix(rapidjson::kNumberType);
        append_();
        bool r = Base::WriteInt(i);
        append_(JsonNodeType::Number);
        return r;
    }
//    bool Uint(unsigned u)       { PrettyPrefix(kNumberType); return Base::WriteUint(u); }
    bool Uint(unsigned u)
    {
        PrettyPrefix(rapidjson::kNumberType);
        append_();
        bool r = Base::WriteUint(u);
        append_(JsonNodeType::Number);
        return r;
    }
//    bool Int64(int64_t i64)     { PrettyPrefix(kNumberType); return Base::WriteInt64(i64); }
    bool Int64(int64_t i64)
    {
        PrettyPrefix(rapidjson::kNumberType);
        append_();
        bool r = Base::WriteInt64(i64);
        append_(JsonNodeType::Number);
        return r;
    }
//    bool Uint64(uint64_t u64)   { PrettyPrefix(kNumberType); return Base::WriteUint64(u64);  }
    bool Uint64(uint64_t u64)
    {
        PrettyPrefix(rapidjson::kNumberType);
        append_();
        bool r = Base::WriteUint64(u64);
        append_(JsonNodeType::Number);
        return r;
    }
    bool Double(double d)
    {
        PrettyPrefix(rapidjson::kNumberType);
        append_();
        bool r = Base::WriteDouble(d);
        append_(JsonNodeType::Number);
        return r;
    }

    bool RawNumber(const Ch* str, rapidjson::SizeType length, bool copy = false)
    {
        (void)copy;
        PrettyPrefix(rapidjson::kNumberType);
        append_();
        bool r = Base::WriteString(str, length);
        append_(JsonNodeType::Number);
        return r;
    }

//    bool String(const Ch* str, SizeType length, bool copy = false) {
//        (void)copy;
//        PrettyPrefix(kStringType);
//        return Base::WriteString(str, length);
//    }
    bool String(const Ch* str, rapidjson::SizeType length, bool copy = false)
    {
        return StringOrKey_(JsonNodeType::String, str, length, copy);
    }

#if RAPIDJSON_HAS_STDSTRING
    bool String(const std::basic_string<Ch>& str) {
        return String(str.data(), SizeType(str.size()));
    }
#endif

//    bool StartObject() {
//        PrettyPrefix(kObjectType);
//        new (Base::level_stack_.template Push<typename Base::Level>()) typename Base::Level(false);
//        return Base::WriteStartObject();
//    }
    bool StartObject()
    {
        PrettyPrefix(rapidjson::kObjectType);
        new (Base::level_stack_.template Push<typename Base::Level>()) typename Base::Level(false);
        bool r = Base::WriteStartObject();
        append_(JsonNodeType::ObjectBegin);
        return r;
    }

//    bool Key(const Ch* str, SizeType length, bool copy = false) { return String(str, length, copy); }
    bool Key(const Ch* str, rapidjson::SizeType length, bool copy = false)
    {
        return StringOrKey_(JsonNodeType::Key, str, length, copy);
    }

#if RAPIDJSON_HAS_STDSTRING
    bool Key(const std::basic_string<Ch>& str) {
        return Key(str.data(), SizeType(str.size()));
    }
#endif

    bool EndObject(rapidjson::SizeType memberCount/* = 0*/)
    {
        (void)memberCount;
        RAPIDJSON_ASSERT(Base::level_stack_.GetSize() >= sizeof(typename Base::Level));
        RAPIDJSON_ASSERT(!Base::level_stack_.template Top<typename Base::Level>()->inArray);
        bool empty = Base::level_stack_.template Pop<typename Base::Level>(1)->valueCount == 0;

        if (!empty) {
            Base::os_->Put('\n');
            WriteIndent();
        }
        bool ret = Base::WriteEndObject();
        (void)ret;
        RAPIDJSON_ASSERT(ret == true);
        if (Base::level_stack_.Empty()) // end of json text
            Base::os_->Flush();
        append_(JsonNodeType::ObjectEnd, memberCount);
        return true;
    }

    bool StartArray()
    {
        PrettyPrefix(rapidjson::kArrayType);
        new (Base::level_stack_.template Push<typename Base::Level>()) typename Base::Level(true);
        bool r = Base::WriteStartArray();
        append_(JsonNodeType::ArrayBegin);
        return r;
    }

    bool EndArray(rapidjson::SizeType memberCount/* = 0*/)
    {
        (void)memberCount;
        RAPIDJSON_ASSERT(Base::level_stack_.GetSize() >= sizeof(typename Base::Level));
        RAPIDJSON_ASSERT(Base::level_stack_.template Top<typename Base::Level>()->inArray);
        bool empty = Base::level_stack_.template Pop<typename Base::Level>(1)->valueCount == 0;

        if (!empty && !(formatOptions_ & rapidjson::kFormatSingleLineArray)) {
            Base::os_->Put('\n');
            WriteIndent();
        }
        bool ret = Base::WriteEndArray();
        (void)ret;
        RAPIDJSON_ASSERT(ret == true);
        if (Base::level_stack_.Empty()) // end of json text
            Base::os_->Flush();
        append_(JsonNodeType::ArrayEnd, memberCount);
        return true;
    }

    //@}

    /*! @name Convenience extensions */
    //@{

    //! Simpler but slower overload.
//    bool String(const Ch* str) { return String(str, internal::StrLen(str)); }
//    bool Key(const Ch* str) { return Key(str, internal::StrLen(str)); }

    //@}

    //! Write a raw JSON value.
    /*!
        For user to write a stringified JSON as a value.

        \param json A well-formed JSON value. It should not contain null character within [0, length - 1] range.
        \param length Length of the json.
        \param type Type of the root of json.
        \note When using PrettyWriter2::RawValue(), the result json may not be indented correctly.
    */
//    bool RawValue(const Ch* json, size_t length, Type type) { PrettyPrefix(type); return Base::WriteRawValue(json, length); }

protected:
    void PrettyPrefix(rapidjson::Type type) {
        (void)type;
        if (Base::level_stack_.GetSize() != 0) { // this value is not at root
            typename Base::Level* level = Base::level_stack_.template Top<typename Base::Level>();

            if (level->inArray) {
                if (level->valueCount > 0) {
                    Base::os_->Put(','); // add comma if it is not the first element in array
                    if (formatOptions_ & rapidjson::kFormatSingleLineArray)
                        Base::os_->Put(' ');
                }

                if (!(formatOptions_ & rapidjson::kFormatSingleLineArray)) {
                    Base::os_->Put('\n');
                    WriteIndent();
                }
            }
            else {  // in object
                if (level->valueCount > 0) {
                    if (level->valueCount % 2 == 0) {
                        Base::os_->Put(',');
                        Base::os_->Put('\n');
                    }
                    else {
                        Base::os_->Put(':');
                        Base::os_->Put(' ');
                    }
                }
                else
                    Base::os_->Put('\n');

                if (level->valueCount % 2 == 0)
                    WriteIndent();
            }
            if (!level->inArray && level->valueCount % 2 == 0)
                RAPIDJSON_ASSERT(type == rapidjson::kStringType);  // if it's in object, then even number should be a name
            level->valueCount++;
        }
        else {
            RAPIDJSON_ASSERT(!Base::hasRoot_);  // Should only has one and only one root.
            Base::hasRoot_ = true;
        }
    }

    void WriteIndent()  {
        size_t count = (Base::level_stack_.GetSize() / sizeof(typename Base::Level)) * indentCharCount_;
        PutN(*Base::os_, static_cast<typename TargetEncoding::Ch>(indentChar_), count);
    }

    Ch indentChar_;
    unsigned indentCharCount_;
    rapidjson::PrettyFormatOptions formatOptions_;

private:
    bool StringOrKey_(JsonNodeType jsonNodeType, const Ch* str, rapidjson::SizeType length, bool isCopy = false)
    {
        (void)isCopy;
        PrettyPrefix(rapidjson::kStringType);
        append_();
        const bool r = Base::WriteString(str, length);
        append_(jsonNodeType);
        return r;
    }
//    LAST: Upgrade to capture/build JSONPath
//    Is it possible to capture begin and end of objects & arrays to highlight background box, like Chrome plugin?
//    Also, that begin/end can be used for nice "jump to..." begin/end feature in text view.
    QString append_()
    {
        const size_t bufferSize = Base::os_->GetSize();
        const typename TargetEncoding::Ch* str = Base::os_->GetString();
        const QString& token = QString::fromUtf8(str + m_lastBufferSize, bufferSize - m_lastBufferSize);
        m_lastBufferSize = bufferSize;
        const int indexOfNewLine = token.indexOf(QLatin1Char{'\n'});
        if (indexOfNewLine >= 0)
        {
            m_currentLine += token.left(indexOfNewLine);
            m_result->jsonTextLineVec.push_back(m_currentLine);

            m_currentLine = token.mid(1 + indexOfNewLine);
            ++m_lineIndex;
        }
        else {
            m_currentLine += token;
        }
        return token;
    }

    const std::unordered_map<JsonNodeType, TextFormat>& m_formatMap;
    size_t m_lastBufferSize;
    QString m_currentLine;

    std::vector<JsonNode*> m_parentNodeVec;
    int m_lineIndex = 0;
    std::unique_ptr<JsonTreeResult> m_result;
    std::unordered_set<QString> internedTextSet;

    /**
     * @param memberCount
     *        -1 unless ending an object or array
     */
    void append_(const JsonNodeType jsonNodeType, const int memberCount = -1)
    {
        // Intentional: Suppress compiler warning for unused args
        (void)memberCount;
        const QString& text = append_();
        std::unordered_map<JsonNodeType, TextFormat>::const_iterator iter = m_formatMap.find(jsonNodeType);
        JsonNode* child = nullptr;
        // Must to set pos.charIndex in logic below.
        TextViewPosition pos{.lineIndex = m_lineIndex};
        switch (jsonNodeType)
        {
            case JsonNodeType::Null:
            case JsonNodeType::Bool:
            case JsonNodeType::Number:
            case JsonNodeType::String: {
                if (nullptr != m_result->jsonTree->rootNode)
                {
                    JsonNode* const parent = m_parentNodeVec.back();
                    if (JsonNodeType::Key == parent->type())
                    {
                        m_parentNodeVec.pop_back();
                    }
                    child = newJsonNode_(parent, jsonNodeType, text);
                }
                else {
                    child = newJsonNode_(nullptr, jsonNodeType, text);
                    m_result->jsonTree->rootNode.reset(child);
                }
                assert(m_formatMap.end() != iter);
                const TextFormat& format = iter->second;
                pos.charIndex = m_currentLine.length() - text.length();
                break;
            }
            case JsonNodeType::Key:
            {
                assert(nullptr != m_result->jsonTree->rootNode);
                JsonNode* const parent = m_parentNodeVec.back();
                assert(JsonNodeType::ObjectBegin == parent->type());
                child = newJsonNode_(parent, jsonNodeType, text);
                m_parentNodeVec.push_back(child);

                assert(m_formatMap.end() != iter);
                const TextFormat& format = iter->second;
                pos.charIndex = m_currentLine.length() - text.length();
                break;
            }
            case JsonNodeType::ObjectBegin:
            {
                assert(m_formatMap.end() == iter);
                if (nullptr != m_result->jsonTree->rootNode) {
                    JsonNode* const parent = m_parentNodeVec.back();
                    if (JsonNodeType::Key == parent->type())
                    {
                        m_parentNodeVec.pop_back();
                    }
                    child = new JsonNode{parent, jsonNodeType, Constants::Json::kObjectBegin};
                    m_parentNodeVec.push_back(child);
                }
                else {
                    child = new JsonNode{nullptr, jsonNodeType, Constants::Json::kObjectBegin};
                    m_result->jsonTree->rootNode.reset(child);
                    m_parentNodeVec.push_back(child);
                }
                assert(QLatin1Char{'{'} == m_currentLine[m_currentLine.length() - 1]);
                pos.charIndex = m_currentLine.length() - 1;
                break;
            }
            case JsonNodeType::ArrayBegin:
            {
                assert(m_formatMap.end() == iter);
                if (nullptr != m_result->jsonTree->rootNode) {
                    JsonNode* const parent = m_parentNodeVec.back();
                    if (JsonNodeType::Key == parent->type())
                    {
                        m_parentNodeVec.pop_back();
                    }
                    child = new JsonNode{parent, jsonNodeType, Constants::Json::kArrayBegin};
                    m_parentNodeVec.push_back(child);
                }
                else {
                    child = new JsonNode{nullptr, jsonNodeType, Constants::Json::kArrayBegin};
                    m_result->jsonTree->rootNode.reset(child);
                    m_parentNodeVec.push_back(child);
                }
                assert(QLatin1Char{'['} == m_currentLine[m_currentLine.length() - 1]);
                pos.charIndex = m_currentLine.length() - 1;
                break;
            }
            case JsonNodeType::ObjectEnd:
            {
                assert(m_formatMap.end() == iter);
                assert(nullptr != m_result->jsonTree->rootNode);
                JsonNode* const parent = m_parentNodeVec.back();
                assert(JsonNodeType::ObjectBegin == parent->type());
                m_parentNodeVec.pop_back();
                // Intentional: We need this final child node to make hide/show logic more simple.
                child = new JsonNode{parent, jsonNodeType, Constants::Json::kObjectEnd};

                assert(QLatin1Char{'}'} == m_currentLine[m_currentLine.length() - 1]);
                pos.charIndex = m_currentLine.length() - 1;
                break;
            }
            case JsonNodeType::ArrayEnd:
            {
                assert(m_formatMap.end() == iter);
                assert(nullptr != m_result->jsonTree->rootNode);
                JsonNode* const parent = m_parentNodeVec.back();
                assert(JsonNodeType::ArrayBegin == parent->type());
                m_parentNodeVec.pop_back();
                // Intentional: We need this final child node to make hide/show logic more simple.
                child = new JsonNode{parent, jsonNodeType, Constants::Json::kArrayEnd};

                assert(QLatin1Char{']'} == m_currentLine[m_currentLine.length() - 1]);
                pos.charIndex = m_currentLine.length() - 1;
                break;
            }
            default:
                assert(false);
        }
        if (m_lineIndex == m_result->jsonTree->lineIndex_To_NodeVec.size())
        {
            std::vector<JsonNode*> nodeVec{};
            // Intentional: Very conservative
            nodeVec.shrink_to_fit();
            m_result->jsonTree->lineIndex_To_NodeVec.push_back(nodeVec);
        }
        if (nullptr != child)
        {
            assert(false == m_result->jsonTree->nodeToPosMap.contains(child));
            Algorithm::Map::insertNewOrAssert(m_result->jsonTree->nodeToPosMap, child, pos);

            std::vector<JsonNode*>& nodeVec = m_result->jsonTree->lineIndex_To_NodeVec.back();
            nodeVec.reserve(nodeVec.size() + 1);
            nodeVec.push_back(child);
        }
    }

    JsonNode*
    newJsonNode_(JsonNode* nullableParent, JsonNodeType type, const QString& text)
    {
//        static int staticCount = 0;
        // See: https://docs.oracle.com/en/java/javase/11/docs/api/java.base/java/lang/String.html#intern()
        std::pair<std::unordered_set<QString>::iterator, bool> insert_pair = internedTextSet.insert(text);
        const QString& internedText = *(insert_pair.first);
//        if (insert_pair.second) {
//            ++staticCount;
//            if (0 == (staticCount % 1000)) {
//                qDebug() << staticCount;
//            }
//        }
        JsonNode* const x = new JsonNode{nullableParent, type, internedText};
        return x;
    }

    // Prohibit copy constructor & assignment operator.
    PrettyWriter2(const PrettyWriter2&);
    PrettyWriter2& operator=(const PrettyWriter2&);
};

//RAPIDJSON_NAMESPACE_END
}  // namespace SDV

#ifdef __GNUC__
RAPIDJSON_DIAG_POP
#endif

#endif // RAPIDJSON_RAPIDJSON_H_
