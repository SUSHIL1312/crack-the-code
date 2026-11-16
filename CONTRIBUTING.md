# Contributing to Crack the Code

Thank you for contributing to **Crack the Code**! Our mission is to maintain an uncompromisingly high-standard, production-quality, self-learning engineering repository for engineers aiming for top-tier technology companies.

---

## 📜 Core Contribution Standards

Every pull request or contribution must satisfy the following strict quality rules:

1. **No Incomplete Stubs or Pseudocode**:
   * Every concept must be accompanied by complete, compilable, and self-contained code.
   * Pseudocode or hand-wavy explanations are strictly prohibited.
2. **Follow the Learn by Doing Template**:
   * `Concept → Internal Working → Visualization → Implementation → Experiment → Interview Questions → Advanced Challenge`.
3. **Verified Claims Only**:
   * Do not invent company-specific claims or attribute interview questions without credible public sources.
   * Reference authoritative documentation (e.g., C++ Core Guidelines, cppreference, Linux man pages, RFCs, Python docs).
4. **Clean Code & Modern Standards**:
   * C++ code must compile under C++20 with `-Wall -Wextra -Wpedantic`.
   * Python code must target Python 3.12+ and pass `ruff` and `mypy`.
5. **No Duplication of Existing Repositories**:
   * Do not duplicate LeetCode DSA solutions here; keep them in [`dsa/leetcode`](dsa/leetcode).
   * Do not duplicate GoF pattern implementations; keep them in [`system-design/low-level-design`](system-design/low-level-design).

---

## 🛠️ Local Development & Testing

### C++ Code Verification
```bash
# Verify compilation across all C++ files
make test-cpp

# Or compile manually with sanitizers:
clang++ -std=c++20 -Wall -Wextra -Wpedantic -fsanitize=address,undefined <file>.cpp -o /tmp/test && /tmp/test
```

### Python Code Verification
```bash
# Run pytest and linters
make test-py
make lint
```

---

## 📝 Pull Request Checklist

Before submitting a PR, ensure:
- [ ] All code files include clear comments explaining tricky operations.
- [ ] Code examples include expected output and time/space complexity analysis.
- [ ] Markdown files use GitHub-flavored markdown and valid internal relative links.
- [ ] Diagrams use Mermaid syntax where appropriate.
- [ ] Solutions to exercises are provided in a dedicated `solutions/` subfolder.
