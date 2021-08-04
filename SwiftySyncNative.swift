import Foundation

enum AuthProvider: Int {
    case facebook = 0
    case google
    case debug
}

class AuthCredentials {
    func getContent() -> String {
        return ""
    }
}

class FacebookCredentials: AuthCredentials {
    
}

class GoogleCredentials: AuthCredentials {
    
}

class DebugCredentials: AuthCredentials {
    
}

struct Field {
    var name: String
    var type: CFieldType
    var value: Any
    
    subscript(key: String) -> Field {
        get {
            if type != cft_array {
                return self
            }
            guard let array = value as? [Field] else { return self }
            return array.first(where: { $0.name == key }) ?? self
        }
        set(newField) {
            if type == cft_array, var array = value as? [Field] {
                let index = array.firstIndex(where: { $0.name == key })
                if (index == nil) {
                    array.append(newField)
                } else {
                    array[index!] = newField
                }
                value = array
            }
        }
    }
    
    init(name: String, value: Bool) {
        self.name = name
        self.value = value
        self.type = cft_boolean
    }
    
    init(name: String, value: Int64) {
        self.name = name
        self.value = value
        self.type = cft_number
    }
    
    init(name: String, value: Double) {
        self.name = name
        self.value = value
        self.type = cft_floatingPoint
    }
    
    init(name: String, value: String) {
        self.name = name
        self.value = value
        self.type = cft_string
    }
    
    init(name: String, children: [Field]) {
        self.name = name
        self.value = children
        self.type = cft_array
    }
}

fileprivate func CFieldToField(cfield: CField) -> Field {
    let fieldName = String(cString: cfield.name)
    switch(cfield.type) {
    case cft_array:
        var children = [Field]()
        for i in 0..<cfield.children_size {
            children.append(CFieldToField(cfield: cfield.children[i]))
        }
        return Field(name: fieldName, children: children)
    case cft_string:
        return Field(name: fieldName, value: String(cString: cfield.str_value))
    case cft_number:
        return Field(name: fieldName, value: cfield.num_value)
    case cft_boolean:
        return Field(name: fieldName, value: cfield.num_value == 1)
    case cft_floatingPoint:
        return Field(name: fieldName, value: cfield.float_value)
    default:
        print("Wrong CField type provided\n")
        return Field(name: "", value: false)
    }
}

fileprivate func FieldToCField(field: Field) -> CField {
    field.name.withCString {
        var result = CField_new(field.type, allocate_string(UnsafeMutablePointer(mutating: $0), field.name.count)).pointee
        switch(field.type) {
        case cft_array:
            let array = field.value as! [Field]
            var children = [CField]()
            for i in 0..<array.count {
                children.append(FieldToCField(field: array[i]))
            }
            children.withUnsafeMutableBytes { ptr in
                result.children = ptr.baseAddress?.bindMemory(to: CField.self, capacity: array.count)
            }
            result.children_size = array.count
        case cft_string:
            let str = field.value as! String
            str.withCString {
                result.str_value = allocate_string(UnsafeMutablePointer(mutating: $0), str.count)
            }
        case cft_number:
            result.num_value = field.value as! Int64
        case cft_boolean:
            result.num_value = field.value as! Bool ? 1 : 0
        case cft_floatingPoint:
            result.float_value = field.value as! Double
        default:
            print("Wrong Field type provided\n")
        }
        return result
    }
}

fileprivate func CFieldArrayToCField(array: CFieldArray) -> CField {
    let result = CField_new(cft_array, "")
    result?.pointee.children = array.ptr
    result?.pointee.children_size = array.size
    return result!.pointee
}

fileprivate func CFieldToCFieldArray(field: CField) -> CFieldArray {
    let result = CFieldArray_new(field.children_size)!
    memcpy(result.pointee.ptr, field.children, CField_size() * field.children_size)
    return result.pointee
}

struct Document {
    var name: String
    var collectionName: String
    var fields: Field
}

fileprivate func makePath(items: [String]) -> String {
    var encoded = "["
    var notFirst = false
    for pathItem in items {
        if(notFirst) {
            encoded += ","
        } else {
            notFirst = true
        }
        encoded += "\"\(pathItem)\""
    }
    return encoded
}

fileprivate func getCField(field: Field) -> UnsafeMutablePointer<CField> {
    let result = CField_new(field.type, field.name)!;
    return result;
}

struct SwiftyNativeClient {
    var id: Int
    
    var isAuthorized: Bool {
        return authorized()
    }
    
    func authorizeUser(provider: AuthProvider, credentials: AuthCredentials) {
        authorize(UInt32(provider.rawValue), credentials.getContent())
    }
    
    func run() {
        run_client()
    }
    
    func callFunction(name: String, bytes: String) -> String {
        guard let cStr = call_function(name, bytes) else { return "" }
        return String(cString: cStr)
    }
    
    func getDocument(collectionName: String, documentName: String) -> Document {
        let arr = CFieldArrayToCField(array: get_document(collectionName, documentName))
        let result = Document(name: documentName, collectionName: collectionName, fields: CFieldToField(cfield: arr))
        return result
    }
    
    func setDocument(_ document: Document) {
        set_document(document.collectionName, document.name, CFieldToCFieldArray(field: FieldToCField(field: document.fields)))
    }
    
    func getField(collectionName: String, documentName: String, path: [String]) -> Field {
        let field = get_field(collectionName, documentName, makePath(items: path)).pointee
        return CFieldToField(cfield: field)
    }
    
    func setField(collectionName: String, documentName: String, path: [String], field: Field) {
        set_field(collectionName, documentName, makePath(items: path), getCField(field: field))
    }
    
    init(uri: String) {
        self.id = 0
        create_client(uri)
    }
}

