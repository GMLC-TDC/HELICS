function v = iterating()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1432107603);
  end
  v = vInitialized;
end
