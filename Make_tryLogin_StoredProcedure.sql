USE [game_server]
GO
/****** Object:  StoredProcedure [dbo].[tryLogin]    Script Date: 2022-04-08 오전 10:13:34 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
-- =============================================
-- Author:		<Author,,Name>
-- Create date: <Create Date,,>
-- Description:	<Description,,>
-- =============================================
ALTER PROCEDURE [dbo].[tryLogin]
	-- Add the parameters for the stored procedure here
	@userID NCHAR(10)
AS
BEGIN
	-- SET NOCOUNT ON added to prevent extra result sets from
	-- interfering with SELECT statements.
	SET NOCOUNT ON;

    -- Insert statements for procedure here
	IF (EXISTS(SELECT userID FROM dbo.userData WHERE userID = @userID))
	BEGIN
	SELECT [level], positionX, positionY, experiencePoint, healthPoint FROM dbo.userData WHERE userID = @userID
	END
	
	ELSE
	BEGIN
	INSERT INTO dbo.userData
	VALUES (@userID, 1, 50, 50, 0, 10)
	SELECT [level], positionX, positionY, experiencePoint, healthPoint FROM dbo.userData WHERE userID = @userID
	END
END
