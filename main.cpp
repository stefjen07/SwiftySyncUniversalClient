#include <GoogleAuthorization.h>
#include <FacebookAuthorization.h>
#include <Authorization.h>
#include <SwiftySyncUniversalClient.h>

using namespace std;

class DebugProvider : public AuthorizationProvider {
public:
	string generateRequest(string body) {
		return "DEBUG";
	}

	DebugProvider() {}
};

int main() {
	SwiftyUniversalClient client("ws://localhost:8888");
	
	auto facebookProvider = FacebookProvider();
	auto castedFacebookProvider = dynamic_cast<AuthorizationProvider*>(&facebookProvider);
	
	auto googleProvider = GoogleProvider();
	auto castedGoogleProvider = dynamic_cast<AuthorizationProvider*>(&googleProvider);

	auto debugProvider = DebugProvider();
	auto castedDebugProvider = dynamic_cast<AuthorizationProvider*>(&debugProvider);
	
	client.supportedProviders = {
		castedFacebookProvider,
		castedGoogleProvider,
		castedDebugProvider
	};

	client.authHandler = [](AuthorizationStatus status) {
		if (status == AuthorizationStatus::authorized) {
			cout << "Authorized successfully\n";
		}
	};

	client.run();

	while (!client.authorized) {
		client.authorize(2, "");
		Sleep(5000);
	}

	Collection usersCollection("users");
	Document doc;
	doc.collection = &usersCollection;
	doc.name = "stefjen07";

	auto testField = Field(FieldType::number, "age");
	testField.numValue = 17;

	doc.fields.push_back(testField);

	client.set_document(doc);

	while (true);
	return 0;
}