function v = iteration_error()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 795176317);
  end
  v = vInitialized;
end
