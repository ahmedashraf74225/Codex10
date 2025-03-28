#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <stack>
#include <algorithm>
#include <sstream>
#include<set>

using namespace std;

// 📩 Message class to handle direct messages
class Message {
public:
    int senderId;   // Unique sender ID
    int receiverId; // Unique receiver ID
    string content; // Message content

    // ✅ Constructor to initialize a message
    Message(int sender, int receiver, string text)
        : senderId(sender), receiverId(receiver), content(text) {}

    // ✅ Display the message in a user-friendly way
    void display() const {
        cout << "From User " << senderId << ": " << content << "\n";
    }
};

// 📝 Post class to represent user posts
class Post {
public:
    static int nextId; // Auto-incrementing post ID
    int id;
    string content;
    int likes;
    vector<string> comments;

    // ✅ Constructor to initialize a post
    Post(string content) : id(nextId++), content(content), likes(0) {}

    void like() { likes++; }  // Increase likes
    void unlike() { if (likes > 0) likes--; } // Decrease likes
    void addComment(string comment) { comments.push_back(comment); } // Add comment

    void display() {
        cout << "Post ID: " << id << "\nContent: " << content << "\nLikes: " << likes << "\nComments:\n";
        for (const auto& comment : comments) cout << " - " << comment << "\n";
        cout << "----------------------\n";
    }
};
int Post::nextId = 1; // Initialize post ID counter
const int MIN_LIKES_TRENDING = 5;

// 👤 User class representing a social media user
class User {
public:
    int id;
    string name;
    vector<Post> posts; // User's own posts
    queue<Message> messages; // Direct messages
    stack<int> postHistory; // Post viewing history
    int engagementScore = 0; // User's engagement level

    User() : id(0), name("") {}

    // ✅ Constructor to initialize a user
    User(int id, string name) : id(id), name(name) {}

    void addPost(string content) { posts.push_back(Post(content)); } // Create a new post
    void sendMessage(Message msg) { messages.push(msg); } // Store incoming message

    // ✅ Display all received messages
    void showMessages() {
        cout << "Messages for " << name << ":\n";
        while (!messages.empty()) {
            messages.front().display();
            messages.pop();
        }
    }

    void increaseEngagement(int amount) { engagementScore += amount; } // Increase engagement score
    void viewPost(int postId) { postHistory.push(postId); } // Store post ID in history

    // ✅ Navigate back to the last viewed post
    void goBackToPreviousPost() {
        if (!postHistory.empty()) {
            cout << name << " is going back to post ID: " << postHistory.top() << "\n";
            postHistory.pop();
        }
        else {
            cout << name << " has no previous posts to go back to.\n";
        }
    }


    void display() {
        cout << "User ID: " << id << ", Name: " << name << endl;
    }


};

// 🌎 Social Media System
class SocialMedia {
private:
    unordered_map<int, User> users; // Store users by ID
    unordered_map<int, Post*> allPosts; // Store posts by ID
    unordered_map<string, vector<int>> keywordMap; // Keyword-to-post index
    priority_queue<pair<int, int>> trendingPosts; // Trending posts (likes, postId)
    unordered_set<int> trendingPostSet; // Track which posts are trending
    set<pair<int, int>, greater<pair<int, int>>> activeUsersSet;

public:
    // ✅ Add a new user
    void addUser(int id, string name) {
        if (users.find(id) != users.end()) {
            cout << "User with ID " << id << " already exists!\n";
            return;
        }
        users[id] = User(id, name);
    }

    // ✅ Create a post and index it by keywords
    void createPost(int userId, string content) {
        if (!users.count(userId)) return;

        users[userId].addPost(content);
        Post& newPost = users[userId].posts.back();
        allPosts[newPost.id] = &newPost;

        // Efficient keyword indexing (avoid duplicates)
        unordered_set<string> uniqueWords;
        stringstream ss(content);
        string word;
        while (ss >> word) {
            uniqueWords.insert(word);
        }
        for (const string& word : uniqueWords) {
            keywordMap[word].push_back(newPost.id);
        }
    }

    void updateActiveUser(int userId, int engagement) {

        // Remove old entry if it exists
        for (auto it = activeUsersSet.begin(); it != activeUsersSet.end(); ++it) {
            if (it->second == userId) {
                activeUsersSet.erase(it);
                break; // Found and removed, exit loop
            }
        }

        // Insert new engagement score
        activeUsersSet.insert({ engagement, userId });
    }

    void likePost(int userId, int postId) {
        if (allPosts.count(postId) && users.count(userId)) {
            allPosts[postId]->like();

            // ✅ Only add to trending if it meets the threshold
            if (allPosts[postId]->likes >= MIN_LIKES_TRENDING &&
                trendingPostSet.find(postId) == trendingPostSet.end()) {

                trendingPosts.push({ allPosts[postId]->likes, postId });
                trendingPostSet.insert(postId);
            }

            // ✅ Increase engagement for the liking user
            users[userId].increaseEngagement(1);

            // Update active user set
            updateActiveUser(userId, users[userId].engagementScore);
        }
    }

    // ✅ Unlike a post
    void unlikePost(int userId, int postId) {
        if (allPosts.count(postId) && users.count(userId)) {
            allPosts[postId]->unlike();

            // Rebuild trending queue if likes drop below threshold
            rebuildTrendingQueue();
        }
    }

    // ✅ Comment on a post
    void commentOnPost(int userId, int postId, string comment) {

        if (allPosts.count(postId) && users.count(userId)) {
            allPosts[postId]->addComment(comment);
            users[userId].increaseEngagement(2);

            // Update active user set
            updateActiveUser(userId, users[userId].engagementScore);
        }
    }

    // ✅ Rebuild trending queue (removing unliked posts)
    void rebuildTrendingQueue() {
        priority_queue<pair<int, int>> newTrendingQueue;
        trendingPostSet.clear();

        for (const auto& postEntry : allPosts) {
            int postId = postEntry.first;
            int likes = postEntry.second->likes;

            if (likes >= 5) {  // Keep only posts with enough likes
                newTrendingQueue.push({ likes, postId });
                trendingPostSet.insert(postId);
            }
        }
        trendingPosts = move(newTrendingQueue);
    }

    // ✅ Binary Search to find a post by its likes
    int binarySearchPostByLikes(int targetLikes) {
        vector<pair<int, int>> sortedPosts;

        // Store posts as (likes, postId) pairs
        for (const auto& p : allPosts) {
            sortedPosts.push_back({ p.second->likes, p.first });
        }

        // Sort posts by likes in ascending order
        sort(sortedPosts.begin(), sortedPosts.end());

        // Perform binary search on the sorted list
        int left = 0, right = sortedPosts.size() - 1;
        while (left <= right) {
            int mid = left + (right - left) / 2;

            if (sortedPosts[mid].first == targetLikes) {
                return sortedPosts[mid].second; // Found post with exact likes
            }
            else if (sortedPosts[mid].first < targetLikes) {
                left = mid + 1; // Search in right half
            }
            else {
                right = mid - 1; // Search in left half
            }
        }
        return -1; // Post with given likes not found
    }

    // ✅ Send a message
    void sendMessage(int senderId, int receiverId, string message) {

        if (users.count(receiverId) && users.count(senderId)) {
            users[receiverId].sendMessage(Message(senderId, receiverId, message));
            users[senderId].increaseEngagement(1);

            // Update active user set
            updateActiveUser(senderId, users[senderId].engagementScore);
        }
    }

    // ✅ Show a user's messages
    void showMessages(int userId) {
        if (users.count(userId)) {
            users[userId].showMessages();
        }
    }

    void displayUsers() {
        cout << "\n--- Users ---\n";
        for (auto& user : users) {
            user.second.display();
        }
    }

    void displayPosts() {
        cout << "\n--- Posts ---\n";
        for (auto& postEntry : allPosts) { // Corrected variable name
            postEntry.second->display();
        }
    }

};

// 🎯 Main Function
int main() {



    SocialMedia app;

    cout << "Adding Users...\n";
    for (int i = 1; i <= 3; i++) {
        app.addUser(i, "User" + to_string(i));
    }
    cout << " Users Added!\n";
    app.displayUsers();

    cout << "\nCreating Posts...\n";
    for (int i = 1; i <= 3; i++) {
        app.createPost(i, "This is post " + to_string(i) + " from User" + to_string(i));
    }
    cout << " Posts Created!\n";
    app.displayPosts();

    cout << "\nLiking Posts...\n";
    for (int i = 1; i <= 3; i++) {
        app.likePost(i, i);
        cout << "User" << i << " liked Post " << i << "\n";
    }
    cout << "Updated Posts:\n";
    app.displayPosts();

    cout << "\nUnliking Posts...\n";
    for (int i = 1; i <= 3; i++) {
        app.unlikePost(i, i);
        cout << "User" << i << " unliked Post " << i << "\n";
    }
    cout << "Updated Posts:\n";
    app.displayPosts();

    cout << "\nCommenting on Posts...\n";
    for (int i = 1; i <= 3; i++) {
        app.commentOnPost(i, (i % 3) + 1, "Nice post, User" + to_string((i % 3) + 1) + "!");
        cout << "User" << i << " commented on Post " << ((i % 3) + 1) << "\n";
    }
    cout << "Updated Posts:\n";
    app.displayPosts();

    cout << "\nSending Messages...\n";
    for (int i = 1; i <= 3; i++) {
        app.sendMessage(i, ((i % 3) + 1), "Hello from User" + to_string(i));
        cout << "User" << i << " sent a message to User" << ((i % 3) + 1) << "\n";
    }
    cout << "Updated Users:\n";
    app.displayUsers();

    cout << "\nDisplaying Messages...\n";
    for (int i = 1; i <= 3; i++) {
        //cout << "Messages for User" << i << ":\n";
        app.showMessages(i);
    }

    cout << "\nSearching for a Post with Likes...\n";
    app.likePost(2, 2);
    app.likePost(3, 3);
    app.likePost(1, 3);


    for (int i = 0; i <= 5; i++) {
        int postId = app.binarySearchPostByLikes(i);
        if (postId != -1)
            cout << "Post with " << i << " likes found: Post ID " << postId << "\n";
        else
            cout << "No post found with " << i << " likes.\n";
    }

    cout << "\n Cycle test completed successfully!\n";


    return 0;
}
