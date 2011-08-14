module Main (main) where

import qualified Control.Monad as CM
import qualified Monad as M
import qualified System as S
import qualified System.Directory as SD
import qualified System.FilePath as SF

findExe :: String -> [SF.FilePath] -> IO (Maybe SF.FilePath)
findExe exeName pathEnvElems =
  CM.foldM
    (\mb -> \dir ->
       case mb of
         Just _ -> return mb
         Nothing ->
           do exist <- SD.doesFileExist $ dir `SF.combine` exeName
              if exist
                then return $ Just dir
                else return Nothing
    )
    Nothing pathEnvElems

main = do let exes = ["ls", "espdiff", "collect2", "collect2"]
          gotSp <- SF.getSearchPath
          let gotSps = [gotSp, gotSp, gotSp ++ ["/usr/lib/gcc/x86_64-linux-gnu/4.5"], gotSp]
          let acts (f:fs) x = do
                M.zipWithM f exes x >>= mapM (maybe
                  (putStrLn "Not found.")
                  putStrLn
                  )
                acts fs x
              acts [] x = return ()
          flip acts gotSps [(\x -> do
            M.zipWithM findExe exes x >>= mapM (maybe
              (putStrLn "Not found.")
              putStrLn
              )
            ), (\x -> do
                putStrLn "Not found."
                return [()]
            )]
