CREATE TABLE user(
	uid varchar(10) NOT NULL,
	passwd char(7) NOT NULL,
	PRIMARY KEY (uid)
);

INSERT INTO user(uid, passwd) VALUES ("testuser", "passwd");
