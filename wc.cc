#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <google/dense_hash_set>
#include <unordered_set>

#include "smart_git.h"

using google::dense_hash_set;
using std::hash;

struct eqstr {
    bool operator()(const char* s1, const char* s2) const
    {
        return (s1 == s2) || (s1 && s2 && strcmp(s1, s2) == 0);
    }
};

typedef dense_hash_set<const char*, hash<const char*>, eqstr> string_hash;

class code_counter {
public:
    code_counter(git_repository *repo)
        : repo_(repo), stats_()
    {
        lines_.set_empty_key(NULL);
    }

    void walk_ref(const char *ref) {
        smart_object<git_commit> commit;
        smart_object<git_tree> tree;
        resolve_ref(commit, ref);
        git_commit_tree(tree, commit);

        walk_tree(tree);
    }
    void dump_stats() {
        printf("Bytes: %ld (dedup: %ld)\n", stats_.bytes, stats_.dedup_bytes);
        printf("Lines: %ld (dedup: %ld)\n", stats_.lines, stats_.dedup_lines);
    }
protected:
    void walk_tree(git_tree *tree) {
        int entries = git_tree_entrycount(tree);
        int i;
        for (i = 0; i < entries; i++) {
            const git_tree_entry *ent = git_tree_entry_byindex(tree, i);
            smart_object<git_object> obj;
            git_tree_entry_2object(obj, repo_, ent);
            if (git_tree_entry_type(ent) == GIT_OBJ_TREE) {
                walk_tree(obj);
            } else if (git_tree_entry_type(ent) == GIT_OBJ_BLOB) {
                update_stats(obj);
            }
        }
    }

    void update_stats(git_blob *blob) {
        char *str;
        size_t len = git_blob_rawsize(blob);
        char *p = new char[len];
        char *end = p + len;
        char *f;
        string_hash::iterator it;
        memcpy(p, git_blob_rawcontent(blob), len);

        while ((f = static_cast<char*>(memchr(p, '\n', end - p))) != 0) {
            *f = '\0';
            it = lines_.find(p);
            if (it == lines_.end()) {
                lines_.insert(p);
                stats_.dedup_bytes += (f - p);
                stats_.dedup_lines ++;
            }
            p = f + 1;
            stats_.lines++;
        }

        stats_.bytes += len;
    }

    void resolve_ref(smart_object<git_commit> &out, const char *refname) {
        git_reference *ref;
        const git_oid *oid;
        git_oid tmp;
        smart_object<git_object> obj;
        if (git_oid_fromstr(&tmp, refname) == GIT_SUCCESS) {
            git_object_lookup(obj, repo_, &tmp, GIT_OBJ_ANY);
        } else {
            git_reference_lookup(&ref, repo_, refname);
            git_reference_resolve(&ref, ref);
            oid = git_reference_oid(ref);
            git_object_lookup(obj, repo_, oid, GIT_OBJ_ANY);
        }
        if (git_object_type(obj) == GIT_OBJ_TAG) {
            git_tag_target(out, obj);
        } else {
            out = obj.release();
        }
    }

    git_repository *repo_;
    string_hash lines_;
    struct {
        unsigned long bytes, dedup_bytes;
        unsigned long lines, dedup_lines;
    } stats_;
};

int main(int argc, char **argv) {
    git_repository *repo;
    git_repository_open(&repo, ".git");

    code_counter counter(repo);

    for (int i = 1; i < argc; i++) {
        printf("Walking %s...\n", argv[i]);
        counter.walk_ref(argv[i]);
    }
    counter.dump_stats();

    return 0;
}