/****** SSMS의 SelectTopNRows 명령 스크립트 ******/
SELECT TOP (1000) [userID]
      ,[level]
      ,[positionX]
      ,[positionY]
      ,[experiencePoint]
      ,[healthPoint]
  FROM [game_server].[dbo].[userData]