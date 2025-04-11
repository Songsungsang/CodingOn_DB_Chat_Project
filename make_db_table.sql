-- chat DB 만들기
CREATE DATABASE chat_db;
USE chat_db;

CREATE TABLE users( -- 회원
	user_id INT PRIMARY KEY AUTO_INCREMENT,
    username VARCHAR(40) UNIQUE,
    password VARCHAR(40),
    status ENUM('Active', 'Suspended') DEFAULT 'Active',
    registered_at DATETIME DEFAULT CURRENT_TIMESTAMP
);
SELECT * FROM users;
DELETE FROM users where user_id IN (3, 6);

CREATE TABLE message_log( -- 채팅기록
	message_id INT PRIMARY KEY AUTO_INCREMENT,
    sender_id INT,
    content VARCHAR(200),
    sent_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    
    CONSTRAINT sender_fk
    FOREIGN KEY (sender_id)
    REFERENCES users(user_id)
    ON DELETE CASCADE -- 삭제했을때 반응
);
SELECT * FROM message_log;

CREATE TABLE user_sessions( -- 로그인, 로그아웃 상태
	session_id INT PRIMARY KEY AUTO_INCREMENT,
    user_id INT,
    login_time DATETIME DEFAULT CURRENT_TIMESTAMP,
    logout_time DATETIME DEFAULT NULL,
    ip_address VARCHAR(40),
    
    CONSTRAINT user_fk
    FOREIGN KEY (user_id)
    REFERENCES users(user_id)
    ON DELETE CASCADE -- 삭제했을때 반응
);

SELECT * FROM user_sessions;

SHOW TABLE STATUS LIKE 'message_log';