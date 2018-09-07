function v = iteration_error()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1329876567);
  end
  v = vInitialized;
end
