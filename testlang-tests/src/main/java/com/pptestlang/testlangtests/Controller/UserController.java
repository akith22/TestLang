package com.pptestlang.testlangtests.Controller;



import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

@RestController
@RequestMapping("/api")
public class UserController {

    @PostMapping("/login")
    public ResponseEntity<String> login(@RequestBody String body) {
        // Return JSON exactly matching test expectations
        String json = "{ \"token\": \"abcd1234\" }";
        return ResponseEntity.ok()
                .header("Content-Type", "application/json")
                .body(json);
    }

    @GetMapping("/users/{id}")
    public ResponseEntity<String> getUser(@PathVariable int id) {
        String json = "{ \"id\": 42, \"username\": \"testuser\", \"role\": \"USER\" }";
        return ResponseEntity.ok()
                .header("Content-Type", "application/json")
                .body(json);
    }

    @PutMapping("/users/{id}")
    public ResponseEntity<String> updateUser(@PathVariable int id, @RequestBody String body) {
        String json = "{ \"updated\": true, \"id\": 42, \"role\": \"ADMIN\" }";
        return ResponseEntity.ok()
                .header("X-App", "TestLangDemo")
                .header("Content-Type", "application/json")
                .body(json);
    }

    @DeleteMapping("/users/{id}")
    public ResponseEntity<String> deleteUser(@PathVariable int id) {
        String json = "{ \"deleted\": true, \"id\": 42 }";
        return ResponseEntity.ok()
                .header("Content-Type", "application/json")
                .body(json);
    }
}
